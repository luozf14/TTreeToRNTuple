#include <tuple>
#include <vector>
#include "TTree.h"
#include "TFile.h"
#include "TRandom3.h"
#include "ROOT/RVec.hxx"
#include "Riostream.h"

void GenTTreeSTLContainer()
{
    auto file = std::make_shared<TFile>("TTreeSTLContainer.root", "RECREATE");
    auto tree = std::make_shared<TTree>("T3", "TTree with STL containers");

    std::vector<Float_t> vec_float;
    std::vector<bool> vec_bool;
    ROOT::RVec<double> RVec_double;
    ROOT::RVec<std::string> RVec_string;
    std::pair<int, float> pair_;
    std::tuple<std::string, int, float> tuple_;
    std::string string_;

    tree->Branch("vec_float","std::vector<Float_t>",&vec_float);
    tree->Branch("vec_bool",&vec_bool);
    tree->Branch("RVec_double",&RVec_double);
    tree->Branch("RVec_string",&RVec_string);
    tree->Branch("pair_", &pair_);
    tree->Branch("tuple_", &tuple_);
    tree->Branch("string_","std::string",&string_);
    
    TRandom3 ranGen;
    Int_t nX;
    for(int i=0; i<2000; i++)
    {
        vec_float.clear();
        vec_bool.clear();
        RVec_double.clear();
        RVec_string.clear();
        nX=(Int_t)(ranGen.Rndm()*10);
        for(int j=0;j<nX;j++) vec_float.push_back(ranGen.Gaus(1,1));
        for(int k=0;k<20;k++) vec_bool.push_back((ranGen.Rndm()*10)>5 ? true:false);
        for(int k=0;k<nX;k++) RVec_double.push_back(ranGen.Rndm()*100);
        for(int k=0;k<5;k++) RVec_string.push_back(std::to_string(ranGen.Rndm()*100));
        pair_ = std::make_pair(i, i * ranGen.Rndm());
        tuple_ = std::make_tuple(std::to_string(i), i, i * ranGen.Rndm() * 100);
        string_ = std::to_string(i);
        
        tree->Fill();
    }

    file->Write();
    file->Close();
}