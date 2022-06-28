#include <ROOT/RField.hxx>
#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleOptions.hxx>

#include <TBranch.h>
#include <TBranchElement.h>

#include <TFile.h>
#include <TLeaf.h>
#include <TROOT.h>
#include <TTree.h>

#include <iostream>
#include <fstream>
#include <memory>
#include <vector>

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
    //    std::unique_ptr<unsigned char []> treeBuffer; // todo (now used for collection fields)
    //    std::unique_ptr<unsigned char []> ntupleBuffer; // todo (now used for collection fields)
    //    unsigned int fldSize;
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
    while ((inputArg = getopt(argc, argv, "hvi:o:t:")) != -1) {
        switch (inputArg) {
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

    std::unique_ptr<TFile> rootFile(TFile::Open(inputFile.c_str()));
    assert(rootFile && !rootFile->IsZombie());

    //
    // Fine the tree in the input root file automatically if not specified in input arguments
    //
    // Works only if the input root file contains only one tree.
    if(!treeNameFlag)treeName=rootFile->GetListOfKeys()->First()->GetName();
    TTree *tree = rootFile->Get<TTree>(treeName.c_str());

    //
    // Get the schema of the tree
    //
    std::vector<FlatField> simpleFields;
    for (auto branch : TRangeDynCast<TBranch>(*tree->GetListOfBranches()))
    {
        assert(branch);
        assert(branch->GetNleaves() == 1);
        TLeaf *leaf = static_cast<TLeaf *>(branch->GetListOfLeaves()->First());
        std::cout << "Detect branch: " << leaf->GetName() << "[" << leaf->GetTypeName() << "]" << std::endl;
        simpleFields.push_back({leaf->GetName(), SanitizeBranchName(leaf->GetName()), leaf->GetTypeName()});
    }

    //
    // Define the RNTuple
    //
    // Create the RNTuple model
    auto model = RNTupleModel::Create();
    for (auto fieldIter : simpleFields)
    {
        auto field = RFieldBase::Create(fieldIter.ntupleName, fieldIter.typeName).Unwrap();
        model->AddField(std::move(field));
        std::cout << "Add field: " << model->GetField(fieldIter.ntupleName)->GetName() << "[" << model->GetField(fieldIter.ntupleName)->GetType() << "]" << std::endl;
    }
    model->Freeze();

    // Bind the tree branch address and the field address
    for (auto fieldIter : simpleFields) 
    {
      // We connect the model's default entry's memory location for the new field to the branch, so that we can
      // fill the ntuple with the data read from the TTree
      void *fieldDataPtr = model->GetDefaultEntry()->GetValue(fieldIter.ntupleName).GetRawPtr();
      tree->SetBranchAddress(fieldIter.treeName.c_str(), fieldDataPtr);
    }

    // Create the RNTuple file
    auto ntuple = RNTupleWriter::Recreate(std::move(model), treeName, outputFile);
    
    // Loop the tree
    auto nEntries = tree->GetEntries();
    for (decltype(nEntries) i = 0; i < nEntries; i++) 
    {
        tree->GetEntry(i);
        ntuple->Fill();
    }

    return 0;
}