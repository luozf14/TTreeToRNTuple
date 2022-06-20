#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RField.hxx>

#include <TBranch.h>
#include <TFile.h>
#include <TLeaf.h>
#include <TROOT.h>
#include <TTree.h>

#include <iostream>
#include <fstream>
#include <memory>
#include <vector>

using RNTupleModel = ROOT::Experimental::RNTupleModel;
using RFieldBase = ROOT::Experimental::Detail::RFieldBase;
using RNTupleReader = ROOT::Experimental::RNTupleReader;
using RNTupleWriter = ROOT::Experimental::RNTupleWriter;

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

struct SimpleField
{
    std::string treeName;
    std::string ntupleName;
    std::string typeName;
    //    std::unique_ptr<unsigned char []> treeBuffer; // todo (now used for collection fields)
    //    std::unique_ptr<unsigned char []> ntupleBuffer; // todo (now used for collection fields)
    //    unsigned int fldSize;
};

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cout << "Error! Please specify the location of input file and output file!" << std::endl;
        std::cout << "Example: ./<executable> input_file output_file" << std::endl;
        return 0;
    }


    char const *inputFile = argv[1];
    std::unique_ptr<TFile> rootFile(TFile::Open(inputFile));
    assert(rootFile && !rootFile->IsZombie());

    //
    // Fine the tree in the input root file.
    //
    // Works only if the input root file contains only one tree.
    std::string treeName = rootFile->GetListOfKeys()->First()->GetName();
    TTree *tree = rootFile->Get<TTree>(treeName.c_str());

    //
    // Get the scheme of the tree
    //
    std::vector<SimpleField> simpleFields;
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

    // Bind the tree branch address and the field address
    for (auto fieldIter : simpleFields) 
    {
      // We connect the model's default entry's memory location for the new field to the branch, so that we can
      // fill the ntuple with the data read from the TTree
      void *fieldDataPtr = model->GetDefaultEntry()->GetValue(fieldIter.ntupleName).GetRawPtr();
      tree->SetBranchAddress(fieldIter.treeName.c_str(), fieldDataPtr);
    }

    // Create the RNTuple file
    char const *kNTupleFileName = argv[2];
    auto ntuple = RNTupleWriter::Recreate(std::move(model), treeName, kNTupleFileName);
    
    // Loop the tree
    auto nEntries = tree->GetEntries();
    for (decltype(nEntries) i = 0; i < nEntries; i++) 
    {
        tree->GetEntry(i);
        ntuple->Fill();
    }

    return 0;
}