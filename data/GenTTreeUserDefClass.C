#include <vector>
#include "TTree.h"
#include "TFile.h"
#include "TRandom3.h"
#include "Riostream.h"
#include "SimpleClass.h"

void GenTTreeUserDefClass()
{
    auto file = std::make_shared<TFile>("TTreeUserDefClass.root", "RECREATE");
    auto tree = std::make_shared<TTree>("T4", "TTree with user defined class");

    SimpleClass simpleClass;

    tree->Branch("simpleClass","SimpleClass",&simpleClass);
    
    TRandom3 ranGen;
    Int_t nX,nY;
    std::vector<Double_t> tempVecDouble;
    std::vector<std::vector<float>> tempVecVecfloat;
    for(int i=0; i<20; i++)
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
        tree->Fill();
    }

    file->Write();
    file->Close();
}