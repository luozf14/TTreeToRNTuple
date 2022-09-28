#include "TTreeToRNTuple.hxx"

#include <string>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <unistd.h>

#include <TFile.h>
#include <TTree.h>
#include <TRandom3.h>

using RNTupleReader = ROOT::Experimental::RNTupleReader;
using ENTupleShowFormat = ROOT::Experimental::ENTupleShowFormat;
using ENTupleInfo = ROOT::Experimental::ENTupleInfo;

void CreateTTree()
{
    auto rootFile = std::make_shared<TFile>("TestFile.root", "RECREATE");
    auto tree = std::make_shared<TTree>("T2", "TTree with c++ arrays");

    Float_t x[3];
    Double_t y[5];
    Int_t nZ;
    Double_t z[10];
    std::array<float, 10> array_float;

    tree->Branch("x", x, "x[3]/F");
    tree->Branch("y", y, "y[5]/D");
    tree->Branch("nZ", &nZ, "nZ/I");
    tree->Branch("z", z, "z[nZ]/D");
    tree->Branch("array_float", &array_float);

    TRandom3 ranGen;
    for (int i = 0; i < 1e5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            if (j < 3)
                x[j] = ranGen.Gaus(0, 1);
            y[j] = ranGen.Rndm(2.);
        }
        nZ = (Int_t)(ranGen.Rndm() * 10);
        for (Int_t k = 0; k < nZ; k++)
        {
            z[k] = ranGen.Rndm();
        }
        for (int k = 0; k < 10; k++)
            array_float[k] = (ranGen.Rndm() * 100);

        tree->Fill();
    }

    rootFile->Write();
    rootFile->Close();
    std::cout << "Created root file containing ttree: T2." << std::endl;
}

void Convert()
{
    std::string inputFile = "TestFile.root";
    std::string outputFile = "TestFile.ntuple";
    std::string treeName = "T2";
    std::string compressionAlgo = "none";
    int compressionLevel = 0;
    std::vector<std::string> dictionary = {};
    std::unique_ptr<TTreeToRNTuple> conversion = std::make_unique<TTreeToRNTuple>(inputFile, outputFile, treeName);
    conversion->SetCompressionAlgoLevel(compressionAlgo, compressionLevel);
    conversion->SetDictionary(dictionary);
    conversion->SetDefaultProgressCallbackFunc();
    conversion->Convert();
}

void View()
{
    std::unique_ptr<TFile> ntupleFile(TFile::Open("TestFile.ntuple"));
    std::string ntupleName = ntupleFile->GetListOfKeys()->First()->GetName();
    auto ntuple = RNTupleReader::Open(ntupleName, "TestFile.ntuple");
    ntuple->PrintInfo();
    int nEntry = 2000;
    printf("The %dth entry is shown below:\n", nEntry);
    ntuple->Show(nEntry, ENTupleShowFormat::kCompleteJSON);
}

void Analyze()
{
    // ttree side
    Float_t x[3];
    Double_t y[5];
    Int_t nZ;
    Double_t z[10];
    std::array<float, 10> array_float;

    auto rootFile = std::make_shared<TFile>("TestFile.root", "READ");
    auto tree = rootFile->Get<TTree>("T2");

    tree->SetBranchAddress("x", &x);
    tree->SetBranchAddress("y", &y);
    tree->SetBranchAddress("nZ", &nZ);
    tree->SetBranchAddress("z", &z);
    tree->SetBranchAddress("array_float", &array_float);

    // rntuple side
    auto model = RNTupleModel::Create();
    auto fldFx = model->MakeField<std::array<float, 3>>("x");
    auto fldDy = model->MakeField<std::array<double, 5>>("y");
    auto fldInZ = model->MakeField<int>("nZ");
    auto fldDz = model->MakeField<std::vector<double>>("z");
    auto fldAarray_float = model->MakeField<std::array<float, 10>>("array_float");

    auto ntuple = RNTupleReader::Open(std::move(model), "T2", "TestFile.ntuple");

    // compare
    for (auto entryId : *ntuple)
    {
        ntuple->LoadEntry(entryId);
        tree->GetEntry(entryId);
        if (!std::equal(x, x + sizeof(x) / sizeof(*x), fldFx->begin()))
            printf("At entry %d, field \"%s\" does not match branch \"%s\"!\n", entryId, "x", "x");
        if (!std::equal(y, y + sizeof(y) / sizeof(*y), fldDy->begin()))
            printf("At entry %d, field \"%s\" does not match branch \"%s\"!\n", entryId, "y", "y");
        if (nZ != *fldInZ)
            printf("At entry %d, field \"%s\" does not match branch \"%s\"!\n", entryId, "nZ", "nZ");
        if (!std::equal(z, z + nZ, fldDz->begin()))
            printf("At entry %d, field \"%s\" does not match branch \"%s\"!\n", entryId, "z", "z");
        if (!std::equal(array_float.begin(), array_float.end(), fldAarray_float->begin()))
            printf("At entry %d, field \"%s\" does not match branch \"%s\"!\n", entryId, "array_float", "array_float");
    }
    std::cout << "Comparison completed!" << std::endl;
}

int main(int argc, char **argv)
{
    CreateTTree();
    Convert();
    // View();
    Analyze();
    return 0;
}