#include "TTreeToRNTuple.hxx"

#include <string>
#include <vector>

using RNTupleReader = ROOT::Experimental::RNTupleReader;
using ENTupleShowFormat = ROOT::Experimental::ENTupleShowFormat;

int main(int argc, char **argv)
{
    std::string inputFile = "../data/TTreeMixed.root";
    std::string outputFile = "output.ntuple";
    std::string treeName = "MixedTree";
    std::string compressionAlgo = "none";
    int compressionLevel = 0;
    std::vector<std::string> dictionary = {"../data/SimpleClass_cxx.so"};
    std::vector<std::string> subBranches = {"simpleClass","x"};
    // bool enableMtiltiThread = false;

    //convert
    std::unique_ptr<TTreeToRNTuple> conversion = std::make_unique<TTreeToRNTuple>(inputFile, outputFile, treeName);
    conversion->SetCompressionAlgoLevel(compressionAlgo, compressionLevel);
    conversion->SetDictionary(dictionary);
    conversion->SetSubBranch(subBranches);
    // conversion->EnableMultiThread(enableMtiltiThread);
    conversion->Convert();

    //view
    std::unique_ptr<TFile> ntupleFile(TFile::Open(outputFile.c_str()));
    std::string ntupleName = ntupleFile->GetListOfKeys()->First()->GetName();
    auto ntuple = RNTupleReader::Open(ntupleName, outputFile);
    ntuple->PrintInfo();
    int nEntry = 200;
    printf("The %dth entry is shown below:\n", nEntry);
    ntuple->Show(nEntry, ENTupleShowFormat::kCompleteJSON);
    
    return 0;
}

   