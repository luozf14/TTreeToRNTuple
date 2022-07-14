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

#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <any>

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
    std::cout << "Usage: " << progname << " -i <input.root> -o <output.ntuple> "
              << "[-t(ree name)]"
              << std::endl;
}

int main(int argc, char **argv)
{
    std::string inputFile = "input.root";
    std::string outputFile = "output.ntuple";
    // int compressionSettings = 0;
    // std::string compressionShorthand = "none";
    // std::string headers;
    // unsigned bloatFactor = 1;
    Bool_t treeNameFlag = false;
    std::string treeName = "tree";

    int inputArg;
    while ((inputArg = getopt(argc, argv, "hvi:o:t:")) != -1)
    {
        switch (inputArg)
        {
        case 'h':
        case 'v':
            Usage(argv[0]);
            return 0;
        case 'i':
            inputFile = optarg;
            break;
        case 'o':
            outputFile = optarg;
            break;
        // case 'c':
        //     compressionSettings = GetCompressionSettings(optarg);
        //     compressionShorthand = optarg;
        //     break;
        // case 'H':
        //     headers = optarg;
        //     break;
        // case 'b':
        //     bloatFactor = std::stoi(optarg);
        //     assert(bloatFactor > 0);
        //     break;
        // case 'm':
        //     ROOT::EnableImplicitMT();
        //     break;
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

    std::unique_ptr<TFile> f(TFile::Open(inputFile.c_str()));
    assert(f && !f->IsZombie());

    //
    // Fine the tree in the input root file automatically if not specified in input arguments
    // Works only if the input root file contains only one tree.
    //
    if (!treeNameFlag)
        treeName = f->GetListOfKeys()->First()->GetName();
    auto tree = f->Get<TTree>(treeName.c_str());

    //
    // Get the scheme of the tree
    //
    std::vector<ContainerField> containerFields;
    for (auto branch : TRangeDynCast<TBranch>(*tree->GetListOfBranches()))
    {
        assert(branch);
        assert(branch->GetNleaves() == 1);

        TLeaf *leaf = static_cast<TLeaf *>(branch->GetListOfLeaves()->First());
        std::cout << "leaf name: " << leaf->GetName() << "; leaf type: " << leaf->GetTypeName() << "; leaf title: " << leaf->GetTitle()
                  << "; leaf length: " << leaf->GetLenStatic() << "; leaf type size: " << leaf->GetLenType() << std::endl;

        containerFields.push_back({leaf->GetName(), SanitizeBranchName(leaf->GetName()), leaf->GetTypeName()});
    }

    auto model = RNTupleModel::CreateBare();
    for (auto &c1 : containerFields)
    {
        auto field = RFieldBase::Create(c1.ntupleName, c1.typeName).Unwrap();
        assert(field);
        model->AddField(std::move(field));
        std::cout << "Add field: " << model->GetField(c1.ntupleName)->GetName() << "; field type name: " << model->GetField(c1.ntupleName)->GetType() << "; field child field type size: "
                  << model->GetField(c1.ntupleName)->GetSubFields()[0]->GetValueSize() << std::endl;
        auto kClass = TClass::GetClass(c1.typeName.c_str());
        c1.treeBuffer = std::make_shared<void *>(nullptr);
        tree->SetBranchAddress(c1.treeName.c_str(), c1.treeBuffer.get(), kClass, EDataType::kOther_t, true);
    }
    model->Freeze();

    auto entry = model->CreateBareEntry();
    for (auto &c1 : containerFields)
    {
        entry->CaptureValueUnsafe(c1.ntupleName, *c1.treeBuffer.get());
    }

    // Create the RNTuple file
    auto ntuple = RNTupleWriter::Recreate(std::move(model), treeName, outputFile);

    // Loop the tree
    auto nEntries = tree->GetEntries();
    for (decltype(nEntries) i = 0; i < nEntries; i++)
    {
        tree->GetEntry(i);
        ntuple->Fill(*entry);
    }

    return 0;
}