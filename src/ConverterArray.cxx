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
    Int_t arrayLength;
    std::map<std::string, void *> fieldAddress;
    void* treeBuffer;   // todo (now used for collection fields)
    void* ntupleBuffer; // todo (now used for collection fields)
    // unsigned int fldSize;
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
    std::vector<FlatField> flatFields;
    for (auto branch : TRangeDynCast<TBranch>(*tree->GetListOfBranches()))
    {
        assert(branch);
        assert(branch->GetNleaves() == 1);

        TLeaf *leaf = static_cast<TLeaf *>(branch->GetListOfLeaves()->First());
        std::cout << "leaf name: " << leaf->GetName() << "; leaf type: " << leaf->GetTypeName() << "; leaf title: " << leaf->GetTitle()
                  << " ; leaf length: " << leaf->GetLenStatic() << std::endl;
        //  If this leaf stores a variable-sized array or a multi-dimensional array whose last dimension has variable size,
        //  return a pointer to the TLeaf that stores such size. Return a nullptr otherwise.
        auto szLeaf = leaf->GetLeafCount();
        if (szLeaf)
        {
            std::cout << szLeaf->GetName() << std::endl;
            std::cout << szLeaf->GetMaximum() << std::endl;
        }
        else
        {
            flatFields.push_back({leaf->GetName(), SanitizeBranchName(leaf->GetName()), leaf->GetTypeName(), leaf->GetLenStatic()});
        }
    }

    auto model = RNTupleModel::Create();
    for (auto &f1 : flatFields)
    {
        std::cout<<"f1.arrayLength: "<<f1.arrayLength<<std::endl;
        if (f1.typeName == "Float_t" && f1.arrayLength>1)
        {
            f1.treeBuffer = new float [f1.arrayLength];
            tree->SetBranchAddress(f1.ntupleName.c_str(), f1.treeBuffer);
            auto field = RFieldBase::Create(f1.ntupleName, "std::vector<float>").Unwrap();
            model->AddField(std::move(field));
        }
        else if (f1.typeName == "Double_t" && f1.arrayLength>1)
        {
            f1.treeBuffer = new double [f1.arrayLength];
            tree->SetBranchAddress(f1.ntupleName.c_str(), f1.treeBuffer);
            auto field = RFieldBase::Create(f1.ntupleName, "std::vector<double>").Unwrap();
            model->AddField(std::move(field));
        }
        std::cout << "Add field: " << model->GetField(f1.ntupleName)->GetName() << "; field type name: " << model->GetField(f1.ntupleName)->GetType() << std::endl;
    }
    model->Freeze();

    for(auto &f1 : flatFields)
    {
        f1.ntupleBuffer = model->GetDefaultEntry()->GetValue(f1.ntupleName).GetRawPtr();
    }

    // Create the RNTuple file
    auto ntuple = RNTupleWriter::Recreate(std::move(model), treeName, outputFile);

    // Loop the tree
    auto nEntries = tree->GetEntries();
    for (decltype(nEntries) i = 0; i < nEntries; ++i)
    {
        tree->GetEntry(i);
        for(auto &f1 : flatFields)
        {
            if(f1.arrayLength>1)
            {
                for(int j=0; j<f1.arrayLength;j++)
                {
                    if(f1.typeName == "Float_t")
                    {
                        ((std::vector<float, std::allocator<float> >*)f1.ntupleBuffer)->push_back(((float *)f1.treeBuffer)[j]);
                    }
                    else if (f1.typeName == "Double_t")
                    {
                        ((std::vector<double, std::allocator<double> >*)f1.ntupleBuffer)->push_back(((double *)f1.treeBuffer)[j]);
                    }
                }
            }
        }
        ntuple->Fill();
    }

    return 0;
}