#include <gtest/gtest.h>

#include "TTreeToRNTuple.hxx"

#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
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

#define nEntries 10000

TEST(UnitTest, CreateTTree)
{
    auto rootFile = std::make_shared<TFile>("/tmp/TestFile.root", "RECREATE");
    auto tree = std::make_shared<TTree>("MixedTree", "TTree containing branches of all types supported by RNTuple");

    gSystem->Load("../../test/SimpleClass_cxx");
    SimpleClass *simpleClass = new SimpleClass();

    Float_t x[3];
    Double_t y[5];
    Int_t nZ;
    Double_t z[nEntries];
    std::array<float, 10> array_float;
    std::vector<Float_t> vec_float;
    std::vector<bool> vec_bool;
    // ROOT::RVec<double> RVec_double;
    // ROOT::RVec<std::string> RVec_string;
    std::pair<int, float> pair_;
    std::tuple<std::string, int, float> tuple_;
    std::string string_;

    tree->Branch("simpleClass", "SimpleClass", &simpleClass);

    tree->Branch("x", x, "x[3]/F");
    tree->Branch("y", y, "y[5]/D");
    tree->Branch("nZ", &nZ, "nZ/I");
    tree->Branch("z", z, "z[nZ]/D");
    tree->Branch("array_float", &array_float);
    tree->Branch("vec_float", "std::vector<Float_t>", &vec_float);
    tree->Branch("vec_bool", &vec_bool);
    // tree->Branch("RVec_double",&RVec_double);
    // tree->Branch("RVec_string",&RVec_string);
    tree->Branch("pair_", &pair_);
    tree->Branch("tuple_", &tuple_);
    tree->Branch("string_", "std::string", &string_);

    Int_t nX, nY;
    std::vector<Double_t> tempVecDouble;
    std::vector<std::vector<float>> tempVecVecfloat;
    for (int i = 0; i < nEntries; i++)
    {
        simpleClass->SetInt(i);
        simpleClass->SetFloat((float)i * 10.);
        nX = (Int_t)(std::log10(i + 1) * 10);
        nY = (Int_t)(std::log10(i + 1) * 10 + 1);
        tempVecDouble.clear();
        tempVecVecfloat.clear();
        for (int j = 0; j < nX; j++)
        {
            tempVecDouble.push_back((double)i);
            std::vector<float> temp;
            for (int k = 0; k < nY; k++)
            {
                temp.push_back((float)i);
            }
            tempVecVecfloat.push_back(temp);
        }
        simpleClass->SetVecDouble(tempVecDouble);
        simpleClass->SetVecVecFloat(tempVecVecfloat);

        for (int j = 0; j < 5; j++)
        {
            if (j < 3)
                x[j] = (float)(i * j);
            y[j] = (double)j;
        }
        nZ = (Int_t)(std::log10(i + 1) * 10 + 2);
        for (Int_t k = 0; k < nZ; k++)
        {
            z[k] = (double)(k * i);
        }
        for (int k = 0; k < 10; k++)
            array_float[k] = (float)(k * i);

        vec_float.clear();
        vec_bool.clear();
        // RVec_double.clear();
        // RVec_string.clear();
        for (int j = 0; j < nX; j++)
            vec_float.push_back((float)(j * i));
        for (int k = 0; k < 20; k++)
            vec_bool.push_back((k * i) % 2 == 0 ? true : false);
        // for (int k = 0; k < nX; k++)
        //     RVec_double.push_back((double)(k * i));
        // for (int k = 0; k < 5; k++)
        //     RVec_string.push_back(std::to_string(k));
        pair_ = std::make_pair(i, (float)i + 1.);
        tuple_ = std::make_tuple(std::to_string(i), i, (float)(i + 100.));
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
    std::string inputFile = "/tmp/TestFile.root";
    std::string outputFile = "/tmp/TestFile.ntuple";
    std::string treeName = "MixedTree";
    std::string compressionAlgo = "lzma";
    int compressionLevel = 9;
    std::vector<std::string> dictionary = {"../../test/SimpleClass_cxx"};
    std::unique_ptr<TTreeToRNTuple> conversion = std::make_unique<TTreeToRNTuple>(inputFile, outputFile, treeName);
    EXPECT_NO_THROW(conversion->SetCompressionAlgoLevel(compressionAlgo, compressionLevel));
    EXPECT_NO_THROW(conversion->SetDictionary(dictionary));
    EXPECT_NO_THROW(conversion->SelectAllBranches());
    EXPECT_NO_THROW(conversion->SetUserProgressCallbackFunc([](int current, int total)
                                                            {
        int interval = total / 100*5;
        if (current % interval == 0)
        {
            fprintf(stderr, "\rProcessing entry %d of %d [\033[00;33m%2.1f%% completed\033[00m]",
                    current, total,
                    (static_cast<float>(current) / total) * 100);
        }
        if(current == total)
        {
            fprintf(stderr, "\rProcessing entry %d of %d [\033[00;32m%2.1f%% completed\033[00m]\n",
                    current, total,
                    (static_cast<float>(current) / total) * 100);
        } }));
    EXPECT_NO_THROW(conversion->Convert(););
}

TEST(UnitTest, Comparison)
{
    // rntuple side
    auto model = RNTupleModel::Create();
    auto fldSimpleClass = model->MakeField<SimpleClass>("simpleClass");

    auto fldFx = model->MakeField<std::array<float, 3>>("x");
    auto fldDy = model->MakeField<std::array<double, 5>>("y");
    auto fldInZ = model->MakeField<int>("nZ");
    auto fldDz = model->MakeField<std::vector<double>>("z");
    auto fldAarray_float = model->MakeField<std::array<float, 10>>("array_float");
    auto fldVec_float = model->MakeField<std::vector<float>>("vec_float");
    auto fldVec_bool = model->MakeField<std::vector<bool>>("vec_bool");
    // auto fldRVec_double = model->MakeField<ROOT::RVec<double>>("RVec_double");
    // auto fldRVec_string = model->MakeField<ROOT::RVec<std::string>>("RVec_string");
    auto fldPair = model->MakeField<std::pair<int, float>>("pair_");
    auto fldTuple = model->MakeField<std::tuple<std::string, int, float>>("tuple_");
    auto fldString = model->MakeField<std::string>("string_");

    auto ntuple = RNTupleReader::Open(std::move(model), "MixedTree", "/tmp/TestFile.ntuple");

    // compare
    Int_t nX, nY, nZ;
    EXPECT_EQ(nEntries, ntuple->GetNEntries()) << "[Number of entries] TTree and RNTuple have different number of entries";
    for (auto entryId : *ntuple)
    {
        ntuple->LoadEntry(entryId);
        nX = (Int_t)(std::log10(entryId + 1) * 10);
        nY = (Int_t)(std::log10(entryId + 1) * 10 + 1);
        nZ = (Int_t)(std::log10(entryId + 1) * 10 + 2);

        // SimpleClass* simpleClass
        EXPECT_EQ(entryId, fldSimpleClass->GetInt()) << "[SimpleClass::GetInt()] Branch 'simpleClass' and field 'simpleClass' differ at entry " << entryId;
        EXPECT_FLOAT_EQ((float)(entryId * 10.), fldSimpleClass->GetFloat()) << "[SimpleClass::GetFloat()] Branch 'simpleClass' and field 'simpleClass' differ at entry " << entryId;
        EXPECT_EQ(nX, fldSimpleClass->GetVecDouble().size()) << "[SimpleClass::GetVecDouble().size()] Branch 'simpleClass' and field 'simpleClass' differ at entry " << entryId;
        for (int j = 0; j < nX; j++)
        {
            EXPECT_DOUBLE_EQ((double)entryId, fldSimpleClass->GetVecDouble().at(j)) << "[SimpleClass::GetVecDouble()] Branch 'simpleClass' and field 'simpleClass' differ at entry " << entryId;
            EXPECT_EQ(nX, fldSimpleClass->GetVecVecFloat().size()) << "[SimpleClass::GetVecVecFloat().size()] Branch 'simpleClass' and field 'simpleClass' differ at entry " << entryId;
            for (int k = 0; k < nY; k++)
            {
                EXPECT_EQ(nY, fldSimpleClass->GetVecVecFloat()[j].size()) << "[SimpleClass::GetVecVecFloat()[j].size()] Branch 'simpleClass' and field 'simpleClass' differ at entry " << entryId;
                EXPECT_FLOAT_EQ((float)entryId, fldSimpleClass->GetVecVecFloat()[j].at(k)) << "[SimpleClass::GetVecVecFloat()[j].at(k)] Branch 'simpleClass' and field 'simpleClass' differ at entry " << entryId;
            }
        }

        // Float_t x[3]
        EXPECT_EQ(3, fldFx->size()) << "[Array length] Branch 'x' and field 'x' differ at entry " << entryId;
        for (decltype(fldFx->size()) i = 0; i < fldFx->size(); i++)
        {
            EXPECT_FLOAT_EQ((float)(entryId * i), fldFx->at(i)) << "Branch 'x' and field 'x' differ at entry " << entryId << " at index" << i;
        }

        // Double_t y[5]
        EXPECT_EQ(5, fldDy->size()) << "[Array length] Branch 'y' and field 'y' differ at entry " << entryId;
        for (decltype(fldDy->size()) i = 0; i < fldDy->size(); i++)
        {
            EXPECT_DOUBLE_EQ((double)i, fldDy->at(i)) << "Branch 'y' and field 'y' differ at entry " << entryId << " at index" << i;
        }

        // Int_t nZ;
        EXPECT_EQ(nZ, *fldInZ) << "Branch 'nZ' and field 'nZ' differ at entry " << entryId;

        // Double_t z[nEntries]
        EXPECT_EQ(nZ, fldDz->size()) << "[Vector length] Branch 'z' and field 'z' differ at entry " << entryId;
        for (decltype(fldDz->size()) i = 0; i < fldDz->size(); i++)
        {
            EXPECT_DOUBLE_EQ((double)(entryId * i), fldDz->at(i)) << "Branch 'z' and field 'z' differ at entry " << entryId << " at index" << i;
        }

        // std::array<float, 10> array_float
        EXPECT_EQ(10, fldAarray_float->size()) << "[Array length] Branch 'array_float' and field 'array_float' differ at entry " << entryId;
        for (decltype(fldAarray_float->size()) i = 0; i < fldAarray_float->size(); i++)
        {
            EXPECT_FLOAT_EQ((float)(entryId * i), fldAarray_float->at(i)) << "Branch 'array_float' and field 'array_float' differ at entry " << entryId << " at index" << i;
        }

        // std::vector<Float_t>* vec_float
        EXPECT_EQ(nX, fldVec_float->size()) << "[Vector length] Branch 'vec_float' and field 'vec_float' differ at entry " << entryId;
        for (decltype(fldVec_float->size()) i = 0; i < fldVec_float->size(); i++)
        {
            EXPECT_FLOAT_EQ((float)(entryId * i), fldVec_float->at(i)) << "Branch 'vec_float' and field 'vec_float' differ at entry " << entryId << " at index" << i;
        }

        // std::vector<bool>* vec_bool
        EXPECT_EQ(20, fldVec_bool->size()) << "[Vector length] Branch 'vec_bool' and field 'vec_bool' differ at entry " << entryId;
        for (decltype(fldVec_bool->size()) i = 0; i < fldVec_bool->size(); i++)
        {
            Bool_t tempBool = (entryId * i) % 2 == 0 ? true : false;
            EXPECT_EQ(tempBool, fldVec_bool->at(i)) << "Branch 'vec_bool' and field 'vec_bool' differ at entry " << entryId << " at index" << i;
        }
        /*
            //ROOT::RVec<double>* RVec_double
            EXPECT_EQ(nX, fldRVec_double->size())<<"[RVec length] Branch 'RVec_double' and field 'RVec_double' differ at entry "<< entryId;
            for(decltype(fldRVec_double->size()) i = 0; i < fldRVec_double->size(); i++)
            {
                EXPECT_DOUBLE_EQ((double)(entryId*i), fldRVec_double->at(i))<< "Branch 'RVec_double' and field 'RVec_double' differ at entry "<<entryId<<" at index" << i;
            }

            //ROOT::RVec<std::string>* RVec_string
            EXPECT_EQ(5, fldRVec_string->size())<<"[RVec length] Branch 'RVec_string' and field 'RVec_string' differ at entry "<< entryId;
            for(decltype(fldRVec_string->size()) i = 0; i < fldRVec_string->size(); i++)
            {
                EXPECT_STREQ(std::to_string(i).c_str(), fldRVec_string->at(i).c_str())<< "Branch 'RVec_string' and field 'RVec_string' differ at entry "<<entryId<<" at index" << i;
            }
        */
        // std::pair<int, float>* pair_
        EXPECT_EQ(entryId, fldPair->first) << "Branch 'pair_' and field 'pair_' differ at entry " << entryId;
        EXPECT_FLOAT_EQ((float)(entryId + 1.), fldPair->second) << "Branch 'pair_' and field 'pair_' differ at entry " << entryId;

        // std::tuple<std::string, int, float>* tuple_
        EXPECT_STREQ(std::to_string(entryId).c_str(), std::get<0>(*fldTuple).c_str()) << "Branch 'tuple_' and field 'tuple_' differ at entry " << entryId;
        EXPECT_EQ(entryId, std::get<1>(*fldTuple)) << "Branch 'pair_' and field 'pair_' differ at entry " << entryId;
        EXPECT_FLOAT_EQ((float)(entryId + 100.), std::get<2>(*fldTuple)) << "Branch 'pair_' and field 'pair_' differ at entry " << entryId;

        // std::string* string_
        EXPECT_STREQ(std::to_string(entryId).c_str(), fldString->c_str()) << "Branch 'string_' and field 'string_' differ at entry " << entryId;
    }
    std::cout << "Comparison completed!" << std::endl;
}
