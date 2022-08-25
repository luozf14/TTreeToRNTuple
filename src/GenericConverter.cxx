#include <ROOT/RField.hxx>
#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleOptions.hxx>

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

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <utility>

#include <unistd.h>

using RCollectionNTupleWriter = ROOT::Experimental::RCollectionNTupleWriter;
using REntry = ROOT::Experimental::REntry;
using RFieldBase = ROOT::Experimental::Detail::RFieldBase;
using RNTupleModel = ROOT::Experimental::RNTupleModel;
using RNTupleWriteOptions = ROOT::Experimental::RNTupleWriteOptions;
using RNTupleWriter = ROOT::Experimental::RNTupleWriter;

// Replace the dot "." by "__" in a name string
static std::string SanitizeBranchName(std::string name)
{
    size_t pos = 0;
    while ((pos = name.find(".", pos)) != std::string::npos)
    {
        name.replace(pos, 1, "__");
        pos += 2;
    }
    return name;
}

struct FlatField
{
    std::string treeName;
    std::string ntupleName;
    std::string typeName;
    Int_t leafTypeSize; // sizeof(leafType)
    Bool_t isVariableSizedArray;
    Int_t arrayLength; // 1 if non-array; size of the array if fixed-length array; maximun size if variable-sized array.
    std::unique_ptr<unsigned char[]> treeBuffer;
    std::unique_ptr<unsigned char[]> ntupleBuffer;
};

struct ContainerField
{
    std::string treeName;
    std::string ntupleName;
    std::string typeName;
    std::shared_ptr<void *> treeBuffer;
    std::unique_ptr<unsigned char[]> ntupleBuffer;
};

static void Usage(char *progname)
{
    std::cout << "Usage: " << progname << " -i <input.root> -o <output.ntuple> [-t(ree) <tree name>] "
              << "[-d(ictionary) <dictionary names>] [-c(ompression) <compression algorithm>] [-m(t)]"
              << std::endl;
}

int GetCompressionSettings(std::string shorthand)
{
    if (shorthand == "zlib")
        return 101;
    if (shorthand == "lz4")
        return 404;
    if (shorthand == "lzma")
        return 207;
    if (shorthand == "zstd")
        return 505;
    if (shorthand == "none")
        return 0;
    abort();
}

int main(int argc, char **argv)
{
    std::string inputFile = "input.root";
    std::string outputFile = "output.ntuple";
    int compressionSettings = 0;
    std::string compressionShorthand = "none";
    std::vector<std::string> dictionaries;
    // unsigned bloatFactor = 1;
    Bool_t treeNameFlag = false;
    std::string treeName = "tree";

    int inputArg;
    while ((inputArg = getopt(argc, argv, "hvi:o:c:d:b:mt:")) != -1)
    {
        switch (inputArg)
        {
        case 'h':
            Usage(argv[0]);
            return 0;
        case 'v':
            Usage(argv[0]);
            return 0;
        case 'i':
            inputFile = optarg;
            break;
        case 'o':
            outputFile = optarg;
            break;
        case 'c':
            compressionSettings = GetCompressionSettings(optarg);
            compressionShorthand = optarg;
            break;
        case 'd':
            dictionaries.push_back(optarg);
            std::cout << "dictionary=" << dictionaries[0] << std::endl;
            break;
        // case 'b':
        //     bloatFactor = std::stoi(optarg);
        //     assert(bloatFactor > 0);
        //     break;
        case 'm':
            ROOT::EnableImplicitMT();
            break;
        case 't':
            treeName = optarg;
            treeNameFlag = true;
            break;
        default:
            fprintf(stderr, "Unknown option: -%c\n", inputArg);
            Usage(argv[0]);
            return 1;
        }
    }

    for (auto d : dictionaries)
    {
        gSystem->Load(d.c_str());
    }

    std::unique_ptr<TFile> file(TFile::Open(inputFile.c_str()));
    assert(file && !file->IsZombie());

    //
    // Fine the tree in the input root file automatically if not specified in input arguments
    // Works only if the input root file contains only one tree.
    //
    if (!treeNameFlag)
        treeName = file->GetListOfKeys()->First()->GetName();
    auto tree = file->Get<TTree>(treeName.c_str());

    //
    // Get the scheme of the tree
    //
    std::vector<FlatField> flatFields;
    std::vector<ContainerField> containerFields;
    for (auto branch : TRangeDynCast<TBranch>(*tree->GetListOfBranches()))
    {
        assert(branch);
        assert(branch->GetNleaves() == 1);

        TLeaf *leaf = static_cast<TLeaf *>(branch->GetListOfLeaves()->First());
        std::cout << "leaf name: " << leaf->GetName() << "; leaf type: " << leaf->GetTypeName() << "; leaf title: " << leaf->GetTitle()
                  << "; leaf length: " << leaf->GetLenStatic() << "; leaf type size: " << leaf->GetLenType() << std::endl;

        if (typeid(*branch) == typeid(TBranchSTL) || typeid(*branch) == typeid(TBranchElement))
        {
            containerFields.push_back({leaf->GetName(), SanitizeBranchName(leaf->GetName()), leaf->GetTypeName()});
        }
        else
        {
            //  If this leaf stores a variable-sized array or a multi-dimensional array whose last dimension has variable size,
            //  return a pointer to the TLeaf that stores such size. Return a nullptr otherwise.
            auto szLeaf = leaf->GetLeafCount();
            if (szLeaf)
            {
                // string treeName, string ntupleName, string typeName, int leafTypeSize, bool isVariableSizedArray, int arrayLength
                flatFields.push_back({leaf->GetName(), SanitizeBranchName(leaf->GetName()), leaf->GetTypeName(), leaf->GetLenType(), kTRUE, szLeaf->GetMaximum()});
            }
            else
            {
                flatFields.push_back({leaf->GetName(), SanitizeBranchName(leaf->GetName()), leaf->GetTypeName(), leaf->GetLenType(), kFALSE, leaf->GetLenStatic()});
            }
        }
    }

    auto model = RNTupleModel::CreateBare();
    for (auto &f1 : flatFields)
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
    for (auto &c1 : containerFields)
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
    for (auto &f1 : flatFields)
    {
        if (f1.isVariableSizedArray)
        {
            f1.ntupleBuffer = std::make_unique<unsigned char[]>(f1.arrayLength * f1.leafTypeSize);
            entry->CaptureValueUnsafe(f1.ntupleName, f1.ntupleBuffer.get());
        }
        else if (!f1.isVariableSizedArray && f1.arrayLength >= 1)
        {
            f1.ntupleBuffer = std::make_unique<unsigned char[]>(f1.arrayLength * f1.leafTypeSize);
            entry->CaptureValueUnsafe(f1.ntupleName, f1.ntupleBuffer.get());
        }
    }
    for (auto &c1 : containerFields)
    {
        entry->CaptureValueUnsafe(c1.ntupleName, *c1.treeBuffer.get());
    }

    // Create the RNTuple file
    RNTupleWriteOptions options;
    options.SetCompression(compressionSettings);
    auto ntuple = RNTupleWriter::Recreate(std::move(model), treeName, outputFile, options);

    // Loop the tree
    auto nEntries = tree->GetEntries();
    for (decltype(nEntries) i = 0; i < nEntries; ++i)
    {
        tree->GetEntry(i);

        for (auto &f1 : flatFields)
        {
            if (f1.isVariableSizedArray)
            {
                Int_t arrayLengthCurrentEntry = tree->GetBranch(f1.treeName.c_str())->GetLeaf(f1.treeName.c_str())->GetLen();
                ((std::vector<unsigned char> *)f1.ntupleBuffer.get())->resize(arrayLengthCurrentEntry * f1.leafTypeSize);
                std::memcpy(((std::vector<unsigned char> *)f1.ntupleBuffer.get())->data(), f1.treeBuffer.get(), arrayLengthCurrentEntry * f1.leafTypeSize);
            }
            if (!f1.isVariableSizedArray && f1.arrayLength >= 1)
            {
                std::memcpy(f1.ntupleBuffer.get(), f1.treeBuffer.get(), f1.arrayLength * f1.leafTypeSize);
            }
        }

        ntuple->Fill(*entry);
    }
    std::cout<<"Conversion completed!"<<std::endl;
    return 0;
}