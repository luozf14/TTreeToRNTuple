#include <ROOT/RField.hxx>
#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleOptions.hxx>
#include <ROOT/RError.hxx>

#include <Compression.h>
#include <TBranch.h>
#include <TFile.h>
#include <TLeaf.h>
#include <TROOT.h>
#include <TTree.h>
#include <TBranchElement.h>
#include <TBranchSTL.h>
#include <TClass.h>
#include <TSystem.h>
#include <TInterpreter.h>

#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <utility>

#include "TTreeToRNTuple.hxx"

using RCollectionNTupleWriter = ROOT::Experimental::RCollectionNTupleWriter;
using REntry = ROOT::Experimental::REntry;
using RFieldBase = ROOT::Experimental::Detail::RFieldBase;
using RNTupleModel = ROOT::Experimental::RNTupleModel;
using RNTupleWriteOptions = ROOT::Experimental::RNTupleWriteOptions;
using RNTupleWriter = ROOT::Experimental::RNTupleWriter;
using RCompressionSetting = ROOT::RCompressionSetting;
using RException = ROOT::Experimental::RException;

TTreeToRNTuple::TTreeToRNTuple(std::string input, std::string output, std::string treeName)
{
    fInputFile = input;
    fOutputFile = output;
    fTreeName = treeName;
    SetCompressionAlgo("none");
}

TTreeToRNTuple::TTreeToRNTuple(std::string input, std::string output, std::string treeName, std::string compressionAlgo, int compressionLevel)
{
    fInputFile = input;
    fOutputFile = output;
    fTreeName = treeName;
    SetCompressionAlgoLevel(compressionAlgo, compressionLevel);
}

TTreeToRNTuple::TTreeToRNTuple(std::string input, std::string output, std::string treeName, std::string compressionAlgo, int compressionLevel, std::vector<std::string> dictionary)
{
    fInputFile = input;
    fOutputFile = output;
    fTreeName = treeName;
    SetCompressionAlgoLevel(compressionAlgo, compressionLevel);
    SetDictionary(dictionary);
}

std::string TTreeToRNTuple::SanitizeBranchName(std::string name)
{
    size_t pos = 0;
    while ((pos = name.find(".", pos)) != std::string::npos)
    {
        name.replace(pos, 1, "__");
        pos += 2;
    }
    return name;
}

void TTreeToRNTuple::SetCompressionAlgo(std::string compressionAlgo)
{
    if (compressionAlgo == "zlib")
    {
        fWriteOptions.SetCompression(101);
    }
    else if (compressionAlgo == "lz4")
    {
        fWriteOptions.SetCompression(404);
    }
    else if (compressionAlgo == "lzma")
    {
        fWriteOptions.SetCompression(207);
    }
    else if (compressionAlgo == "zstd")
    {
        fWriteOptions.SetCompression(505);
    }
    else if (compressionAlgo == "none")
    {
        fWriteOptions.SetCompression(0);
    }
    else
    {
        abort();
    }
}

void TTreeToRNTuple::SetCompressionAlgoLevel(std::string compressionAlgo, int compressionLevel)
{
    if (compressionAlgo == "zlib")
    {
        fWriteOptions.SetCompression(RCompressionSetting::EAlgorithm::EValues::kZLIB, compressionLevel);
    }
    else if (compressionAlgo == "lz4")
    {
        fWriteOptions.SetCompression(RCompressionSetting::EAlgorithm::EValues::kLZ4, compressionLevel);
    }
    else if (compressionAlgo == "lzma")
    {
        fWriteOptions.SetCompression(RCompressionSetting::EAlgorithm::EValues::kLZMA, compressionLevel);
    }
    else if (compressionAlgo == "zstd")
    {
        fWriteOptions.SetCompression(RCompressionSetting::EAlgorithm::EValues::kZSTD, compressionLevel);
    }
    else if (compressionAlgo == "none")
    {
        fWriteOptions.SetCompression(RCompressionSetting::EAlgorithm::EValues::kUseGlobal, compressionLevel);
    }
    else
    {
        abort();
    }
}

void TTreeToRNTuple::SetDictionary(std::vector<std::string> dictionary)
{
    for (auto d : dictionary)
    {
        int loadStatus = gSystem->Load(d.c_str());
        if (loadStatus == 0 || loadStatus == 1)
        {
            printf("Load dictionary \"%s\" successfully!\n", d.c_str());
        }
        else
        {
            printf("Error: Load dictionary \"%s\" unsuccessfully! Load status: %d\n", d.c_str(), loadStatus);
            exit(0);
        }
    }
}

void TTreeToRNTuple::SelectBranches(std::vector<std::string> subBranch)
{
    for (auto sb : subBranch)
    {
        fSelectedBranches.push_back(SanitizeBranchName(sb));
    }
}

void TTreeToRNTuple::SelectAllBranches()
{
    fSelectedBranches = {};
}

void TTreeToRNTuple::SetTreeName(std::string treeName)
{
    fTreeName = treeName;
}

void TTreeToRNTuple::SetInputFile(std::string input)
{
    fInputFile = input;
}

void TTreeToRNTuple::SetOutputFile(std::string output)
{
    fOutputFile = output;
}

void TTreeToRNTuple::Convert(std::unique_ptr<ProgressListener> listener)
{
    std::unique_ptr<TFile> file(TFile::Open(fInputFile.c_str()));
    assert(file && !file->IsZombie());

    auto tree = file->Get<TTree>(fTreeName.c_str());
    if (!tree)
    {
        throw RException(R__FAIL("Tree \"" + fTreeName + "\" is not found!\n"));
    }

    Long_t nEntries = tree->GetEntries();
    printf("Number of entries in tree \"%s\": %ld.\n", fTreeName.c_str(), nEntries);

    //
    // Get the scheme of the tree
    //
    for (auto branch : TRangeDynCast<TBranch>(*tree->GetListOfBranches()))
    {
        assert(branch);
        assert(branch->GetNleaves() == 1);
        if (!fSelectedBranches.empty() && std::find(fSelectedBranches.begin(), fSelectedBranches.end(), SanitizeBranchName(branch->GetName())) == fSelectedBranches.end())
        {
            continue;
        }

        TLeaf *leaf = static_cast<TLeaf *>(branch->GetListOfLeaves()->First());
        std::cout << "In root file detect leaf name: " << leaf->GetName() << "; leaf type: " << leaf->GetTypeName() << "; leaf title: " << leaf->GetTitle()
                  << "; leaf length: " << leaf->GetLenStatic() << "; leaf type size: " << leaf->GetLenType() << std::endl;

        if (typeid(*branch) == typeid(TBranchSTL) || typeid(*branch) == typeid(TBranchElement))
        {
            fContainerFields.push_back({leaf->GetName(), SanitizeBranchName(leaf->GetName()), leaf->GetTypeName()});
        }
        else
        {
            //  If this leaf stores a variable-sized array or a multi-dimensional array whose last dimension has variable size,
            //  return a pointer to the TLeaf that stores such size. Return a nullptr otherwise.
            auto szLeaf = leaf->GetLeafCount();
            if (szLeaf)
            {
                // string treeName, string ntupleName, string typeName, int leafTypeSize, bool isVariableSizedArray, int arrayLength
                fFlatFields.push_back({leaf->GetName(), SanitizeBranchName(leaf->GetName()), leaf->GetTypeName(), leaf->GetLenType(), kTRUE, szLeaf->GetMaximum()});
            }
            else
            {
                fFlatFields.push_back({leaf->GetName(), SanitizeBranchName(leaf->GetName()), leaf->GetTypeName(), leaf->GetLenType(), kFALSE, leaf->GetLenStatic()});
            }
        }
    }

    auto model = RNTupleModel::CreateBare();
    for (auto &f1 : fFlatFields)
    {
        if (f1.isVariableSizedArray)
        {
            auto field = RFieldBase::Create(f1.ntupleName, "std::vector<" + f1.typeName + ", " + std::to_string(f1.arrayLength) + ">").Unwrap();
            assert(field);
            model->AddField(std::move(field));
            std::cout << "Add field: " << model->GetField(f1.ntupleName)->GetName() << "; field type name: " << model->GetField(f1.ntupleName)->GetType() << std::endl;
            f1.treeBuffer = std::make_unique<unsigned char[]>(f1.arrayLength * f1.leafTypeSize);
            tree->SetBranchAddress(f1.ntupleName.c_str(), (void *)f1.treeBuffer.get());
        }
        else if (!f1.isVariableSizedArray && f1.arrayLength > 1)
        {
            auto field = RFieldBase::Create(f1.ntupleName, "std::array<" + f1.typeName + ", " + std::to_string(f1.arrayLength) + ">").Unwrap();
            assert(field);
            model->AddField(std::move(field));
            std::cout << "Add field: " << model->GetField(f1.ntupleName)->GetName() << "; field type name: " << model->GetField(f1.ntupleName)->GetType() << std::endl;
            f1.treeBuffer = std::make_unique<unsigned char[]>(f1.arrayLength * f1.leafTypeSize);
            tree->SetBranchAddress(f1.ntupleName.c_str(), (void *)f1.treeBuffer.get());
        }
        else if (!f1.isVariableSizedArray && f1.arrayLength == 1)
        {
            auto field = RFieldBase::Create(f1.ntupleName, f1.typeName).Unwrap();
            assert(field);
            model->AddField(std::move(field));
            std::cout << "Add field: " << model->GetField(f1.ntupleName)->GetName() << "; field type name: " << model->GetField(f1.ntupleName)->GetType() << std::endl;
            f1.treeBuffer = std::make_unique<unsigned char[]>(f1.arrayLength * f1.leafTypeSize);
            tree->SetBranchAddress(f1.ntupleName.c_str(), (void *)f1.treeBuffer.get());
        }
    }
    for (auto &c1 : fContainerFields)
    {
        auto field = RFieldBase::Create(c1.ntupleName, c1.typeName).Unwrap();
        assert(field);
        model->AddField(std::move(field));
        std::cout << "Add field: " << model->GetField(c1.ntupleName)->GetName() << "; field type name: " << model->GetField(c1.ntupleName)->GetType() << std::endl;
        auto kClass = TClass::GetClass(c1.typeName.c_str());
        c1.treeBuffer = std::make_shared<void *>(nullptr);
        tree->SetBranchAddress(c1.treeName.c_str(), c1.treeBuffer.get(), kClass, EDataType::kOther_t, true);
    }
    model->Freeze();

    auto entry = model->CreateBareEntry();
    for (auto &f1 : fFlatFields)
    {
        // f1.ntupleBuffer = std::make_unique<unsigned char[]>(f1.arrayLength * f1.leafTypeSize);
        // entry->CaptureValueUnsafe(f1.ntupleName, f1.ntupleBuffer.get());
        if (f1.isVariableSizedArray)
        {
            f1.ntupleBuffer = std::make_unique<unsigned char[]>(f1.arrayLength * f1.leafTypeSize);
            entry->CaptureValueUnsafe(f1.ntupleName, f1.ntupleBuffer.get());
            // std::cout << "f1.isVariableSizedArray" << std::endl;
        }
        else if (!f1.isVariableSizedArray && f1.arrayLength >= 1)
        {
            // f1.ntupleBuffer = std::make_unique<unsigned char[]>(f1.arrayLength * f1.leafTypeSize);
            entry->CaptureValueUnsafe(f1.ntupleName, f1.treeBuffer.get());
            // std::cout << "!f1.isVariableSizedArray && f1.arrayLength >= 1" << std::endl;
        }
    }
    for (auto &c1 : fContainerFields)
    {
        entry->CaptureValueUnsafe(c1.ntupleName, *c1.treeBuffer.get());
        // std::cout << "container" << std::endl;
    }

    // Create the RNTuple file
    auto ntuple = RNTupleWriter::Recreate(std::move(model), fTreeName, fOutputFile, fWriteOptions);

    // Loop the tree
    for (decltype(nEntries) i = 0; i < nEntries; i++)
    {
        tree->GetEntry(i);

        for (auto &f1 : fFlatFields)
        {
            if (f1.isVariableSizedArray)
            {
                Int_t arrayLengthCurrentEntry = tree->GetBranch(f1.treeName.c_str())->GetLeaf(f1.treeName.c_str())->GetLen();
                ((std::vector<unsigned char> *)f1.ntupleBuffer.get())->resize(arrayLengthCurrentEntry * f1.leafTypeSize);
                std::memcpy(((std::vector<unsigned char> *)f1.ntupleBuffer.get())->data(), f1.treeBuffer.get(), arrayLengthCurrentEntry * f1.leafTypeSize);
            }
            // if (!f1.isVariableSizedArray && f1.arrayLength >= 1)
            // {
            //     std::memcpy(f1.ntupleBuffer.get(), f1.treeBuffer.get(), f1.arrayLength * f1.leafTypeSize);
            // }
        }

        ntuple->Fill(*entry);
        if (listener)
        {
            listener->Notify(i, nEntries);
        }
    }
    if (listener)
    {
        listener->NotifyComplete(nEntries);
    }
}