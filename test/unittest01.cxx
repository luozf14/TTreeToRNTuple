#include <gtest/gtest.h>

#include "TTreeToRNTuple.hxx"

#include <string>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <unistd.h>

#include <TFile.h>
#include <TTree.h>
#include <TRandom3.h>
#include <TSystem.h>
#include <TROOT.h>
#include "ROOT/RVec.hxx"

#include "SimpleClass.h"

using RNTupleReader = ROOT::Experimental::RNTupleReader;
using ENTupleShowFormat = ROOT::Experimental::ENTupleShowFormat;
using ENTupleInfo = ROOT::Experimental::ENTupleInfo;

TEST(UnitTest, CreateTTree)
{
    auto rootFile = std::make_shared<TFile>("TestFile.root", "RECREATE");
    auto tree = std::make_shared<TTree>("MixedTree", "TTree containing branches of all types supported by RNTuple");

    gSystem->Load("../../test/SimpleClass_cxx");
    SimpleClass* simpleClass = new SimpleClass();

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
    for(int i=0; i<9e5; i++)
    {
        simpleClass->SetInt(i);
        simpleClass->SetFloat(ranGen.Rndm()*10);
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
        simpleClass->SetVecDouble(tempVecDouble);
        simpleClass->SetVecVecFloat(tempVecVecfloat);

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
    rootFile->Write();
    rootFile->Close();
    delete simpleClass;
    std::cout << "Created root file containing ttree: MixedTree." << std::endl;

}

TEST(UnitTest, Conversion)
{
    std::string inputFile = "TestFile.root";
    std::string outputFile = "TestFile.ntuple";
    std::string treeName = "MixedTree";
    std::string compressionAlgo = "none";
    int compressionLevel = 0;
    std::vector<std::string> dictionary = {"../../test/SimpleClass_cxx"};
    std::unique_ptr<TTreeToRNTuple> conversion = std::make_unique<TTreeToRNTuple>(inputFile, outputFile, treeName);
    conversion->SetCompressionAlgoLevel(compressionAlgo, compressionLevel);
    conversion->SetDictionary(dictionary);
    conversion->SelectAllBranches();
    conversion->SetDefaultProgressCallbackFunc();
    EXPECT_NO_THROW(conversion->Convert(););
}

TEST(UnitTest, Comparison)
{
    // ttree side
    gSystem->Load("../../test/SimpleClass_cxx");
    SimpleClass* simpleClass = new SimpleClass();
    Float_t x[3];
    Double_t y[5];
    Int_t nZ;
    Double_t z[10];
    std::array<float, 10> array_float;
    std::vector<Float_t>* vec_float = nullptr;
    std::vector<bool>* vec_bool = nullptr;
    ROOT::RVec<double>* RVec_double = nullptr;
    ROOT::RVec<std::string>* RVec_string = nullptr;
    std::pair<int, float>* pair_ = nullptr;
    std::tuple<std::string, int, float>* tuple_ = nullptr;
    std::string* string_ = nullptr;
    auto rootFile = std::make_shared<TFile>("TestFile.root", "READ");
    auto tree = rootFile->Get<TTree>("MixedTree");
    tree->SetBranchAddress("simpleClass",&simpleClass);
    tree->SetBranchAddress("x", &x);
    tree->SetBranchAddress("y", &y);
    tree->SetBranchAddress("nZ", &nZ);
    tree->SetBranchAddress("z", &z);
    tree->SetBranchAddress("array_float",&array_float);
    tree->SetBranchAddress("vec_float",&vec_float);
    tree->SetBranchAddress("vec_bool",&vec_bool);
    tree->SetBranchAddress("RVec_double",&RVec_double);
    tree->SetBranchAddress("RVec_string",&RVec_string);
    tree->SetBranchAddress("pair_", &pair_);
    tree->SetBranchAddress("tuple_", &tuple_);
    tree->SetBranchAddress("string_",&string_);

    // rntuple side
    auto model = RNTupleModel::Create();
    auto fldFx = model->MakeField<std::array<float, 3>>("x");
    auto fldDy = model->MakeField<std::array<double, 5>>("y");
    auto fldInZ = model->MakeField<int>("nZ");
    auto fldDz = model->MakeField<std::vector<double>>("z");
    auto fldAarray_float = model->MakeField<std::array<float, 10>>("array_float");
    auto fldVec_float = model->MakeField<std::vector<float>>("vec_float");
    auto fldVec_bool = model->MakeField<std::vector<bool>>("vec_bool");
    auto fldRVec_double = model->MakeField<ROOT::RVec<double>>("RVec_double");
    auto fldRVec_string = model->MakeField<ROOT::RVec<std::string>>("RVec_string");
    auto fldPair = model->MakeField<std::pair<int, float>>("pair_");
    auto fldTuple = model->MakeField<std::tuple<std::string, int, float>>("tuple_");
    auto fldString = model->MakeField<std::string>("string_");
    auto fldSimpleClass = model->MakeField<SimpleClass>("simpleClass");

    auto ntuple = RNTupleReader::Open(std::move(model), "MixedTree", "TestFile.ntuple");
    
    // compare
    for (auto entryId : *ntuple)
    {
        ntuple->LoadEntry(entryId);
        tree->GetEntry(entryId);

        //Float_t x[3]
        EXPECT_EQ(sizeof(x) / sizeof(*x), fldFx->size())<<"[Array length] Branch 'x' and field 'x' differ at entry "<< entryId;
        for(decltype(fldFx->size()) i = 0; i < fldFx->size(); i++)
        {
            EXPECT_FLOAT_EQ(x[i], fldFx->at(i))<< "Branch 'x' and field 'x' differ at entry "<<entryId<<" at index" << i;
        }

        //Double_t y[5] 
        EXPECT_EQ(sizeof(y) / sizeof(*y), fldDy->size())<<"[Array length] Branch 'y' and field 'y' differ at entry "<< entryId;
        for(decltype(fldDy->size()) i = 0; i < fldDy->size(); i++)
        {
            EXPECT_DOUBLE_EQ(y[i], fldDy->at(i))<< "Branch 'y' and field 'y' differ at entry "<<entryId<<" at index" << i;
        }

        //Int_t nZ;
        EXPECT_EQ(nZ, *fldInZ)<<"Branch 'nZ' and field 'nZ' differ at entry "<< entryId;

        //Double_t z[10]
        EXPECT_EQ(nZ, fldDz->size())<<"[Vector length] Branch 'z' and field 'z' differ at entry "<< entryId;
        for(decltype(fldDz->size()) i = 0; i < fldDz->size(); i++)
        {
            EXPECT_DOUBLE_EQ(z[i], fldDz->at(i))<< "Branch 'z' and field 'z' differ at entry "<<entryId<<" at index" << i;
        }

        //std::array<float, 10> array_float
        EXPECT_EQ(array_float.size(), fldAarray_float->size())<<"[Array length] Branch 'array_float' and field 'array_float' differ at entry "<< entryId;
        for(decltype(fldAarray_float->size()) i = 0; i < fldAarray_float->size(); i++)
        {
            EXPECT_FLOAT_EQ(array_float.at(i), fldAarray_float->at(i))<< "Branch 'array_float' and field 'array_float' differ at entry "<<entryId<<" at index" << i;
        }

        //std::vector<Float_t>* vec_float
        EXPECT_EQ(vec_float->size(), fldVec_float->size())<<"[Vector length] Branch 'vec_float' and field 'vec_float' differ at entry "<< entryId;
        for(decltype(fldVec_float->size()) i = 0; i < fldVec_float->size(); i++)
        {
            EXPECT_FLOAT_EQ(vec_float->at(i), fldVec_float->at(i))<< "Branch 'vec_float' and field 'vec_float' differ at entry "<<entryId<<" at index" << i;
        }

        //std::vector<bool>* vec_bool
        EXPECT_EQ(vec_bool->size(), fldVec_bool->size())<<"[Vector length] Branch 'vec_bool' and field 'vec_bool' differ at entry "<< entryId;
        for(decltype(fldVec_bool->size()) i = 0; i < fldVec_bool->size(); i++)
        {
            EXPECT_EQ(vec_bool->at(i), fldVec_bool->at(i))<< "Branch 'vec_float' and field 'vec_float' differ at entry "<<entryId<<" at index" << i;
        }

        //ROOT::RVec<double>* RVec_double
        EXPECT_EQ(RVec_double->size(), fldRVec_double->size())<<"[RVec length] Branch 'RVec_double' and field 'RVec_double' differ at entry "<< entryId;
        for(decltype(fldRVec_double->size()) i = 0; i < fldRVec_double->size(); i++)
        {
            EXPECT_DOUBLE_EQ(RVec_double->at(i), fldRVec_double->at(i))<< "Branch 'RVec_double' and field 'RVec_double' differ at entry "<<entryId<<" at index" << i;
        }

        //ROOT::RVec<std::string>* RVec_string
        EXPECT_EQ(RVec_string->size(), fldRVec_string->size())<<"[RVec length] Branch 'RVec_string' and field 'RVec_string' differ at entry "<< entryId;
        for(decltype(fldRVec_string->size()) i = 0; i < fldRVec_string->size(); i++)
        {
            EXPECT_STREQ(RVec_string->at(i).c_str(), fldRVec_string->at(i).c_str())<< "Branch 'RVec_string' and field 'RVec_string' differ at entry "<<entryId<<" at index" << i;
        }

        //std::pair<int, float>* pair_
        EXPECT_EQ(*pair_, *fldPair)<<"Branch 'pair_' and field 'pair_' differ at entry "<< entryId;

        //std::tuple<std::string, int, float>* tuple_
        EXPECT_EQ(*tuple_, *fldTuple)<<"Branch 'tuple_' and field 'tuple_' differ at entry "<< entryId;

        //std::string* string_
        EXPECT_STREQ((*string_).c_str(), fldString->c_str())<<"Branch 'string_' and field 'string_' differ at entry "<< entryId;

        //SimpleClass* simpleClass
        EXPECT_EQ(simpleClass->GetInt(), fldSimpleClass->GetInt())<<"[SimpleClass::GetInt()] Branch 'simpleClass' and field 'simpleClass' differ at entry "<< entryId;
        EXPECT_FLOAT_EQ(simpleClass->GetFloat(), fldSimpleClass->GetFloat())<<"[SimpleClass::GetFloat()] Branch 'simpleClass' and field 'simpleClass' differ at entry "<< entryId;
        EXPECT_EQ(simpleClass->GetVecDouble(), fldSimpleClass->GetVecDouble())<<"[SimpleClass::GetVecDouble()] Branch 'simpleClass' and field 'simpleClass' differ at entry "<< entryId;
        EXPECT_EQ(simpleClass->GetVecVecFloat(), fldSimpleClass->GetVecVecFloat())<<"[SimpleClass::GetVecVecFloat()] Branch 'simpleClass' and field 'simpleClass' differ at entry "<< entryId;
        
    }
    delete simpleClass;
    std::cout << "Comparison completed!" << std::endl;
}

