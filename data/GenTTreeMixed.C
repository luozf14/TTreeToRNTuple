#include <tuple>
#include <vector>
#include "TTree.h"
#include "TFile.h"
#include "TRandom3.h"
#include "Riostream.h"
#include "ROOT/RVec.hxx"
#include "SimpleClass.h"

void GenTTreeMixed()
{
    auto file = std::make_shared<TFile>("TTreeMixed.root", "RECREATE");
    auto tree = std::make_shared<TTree>("MixedTree", "TTree with user defined class");

    // gSystem->Load("SimpleClass_cxx");

    SimpleClass simpleClass;

    Float_t x[3];
    Double_t y[5];
    Int_t nZ;
    Double_t z[10];
    std::array<float, 10> array_float;
    std::vector<Float_t> vec_float;
    std::vector<bool> vec_bool;
    ROOT::RVec<double> RVec_double;
    ROOT::RVec<std::string> RVec_string;
    std::pair<int, float> pair_;
    std::tuple<std::string, int, float> tuple_;
    std::string string_;

    tree->Branch("simpleClass","SimpleClass",&simpleClass);

    tree->Branch("x", x, "x[3]/F");
    tree->Branch("y", y, "y[5]/D");
    tree->Branch("nZ", &nZ, "nZ/I");
    tree->Branch("z", z, "z[nZ]/D");
    tree->Branch("array_float",&array_float);
    tree->Branch("vec_float","std::vector<Float_t>",&vec_float);

    tree->Branch("vec_bool",&vec_bool);
    tree->Branch("RVec_double",&RVec_double);
    tree->Branch("RVec_string",&RVec_string);
    tree->Branch("pair_", &pair_);
    tree->Branch("tuple_", &tuple_);
    tree->Branch("string_","std::string",&string_);

    TRandom3 ranGen;
    Int_t nX,nY;
    std::vector<Double_t> tempVecDouble;
    std::vector<std::vector<float>> tempVecVecfloat;
    for(int i=0; i<2000; i++)
    {
        simpleClass.SetInt(i);
        simpleClass.SetFloat(ranGen.Rndm()*10);
        nX=(Int_t)(ranGen.Rndm()*10);
        nY=(Int_t)(ranGen.Rndm()*10);
        tempVecDouble.clear();
        tempVecVecfloat.clear();
        for(int j=0;j<nX;j++) 
        {
            tempVecDouble.push_back(ranGen.Gaus());
            std::vector<float> temp;
            for (int k = 0; k < nY; k++)
            {
                temp.push_back(ranGen.Rndm());
            }
            tempVecVecfloat.push_back(temp);
        }
        simpleClass.SetVecDouble(tempVecDouble);
        simpleClass.SetVecVecFloat(tempVecVecfloat);

        for(int j=0;j<5;j++)
        {
            if(j<3) x[j]=ranGen.Gaus(0,1);
            y[j]=ranGen.Rndm(2.);
        }
        nZ=(Int_t)(ranGen.Rndm()*10);
        for(Int_t k=0;k<nZ;k++)
        {
            z[k]=ranGen.Rndm();
        }
        for(int k=0;k<10;k++) array_float[k]=(ranGen.Rndm()*100);

        vec_float.clear();
        vec_bool.clear();
        RVec_double.clear();
        RVec_string.clear();
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