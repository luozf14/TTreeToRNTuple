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

    tree->Branch("SimpleClass","SimpleClass",&simpleClass);
    
    TRandom3 ranGen;
    Int_t nX;
    std::vector<Double_t> tempVecDouble;
    for(int i=0; i<20; i++)
    {
        simpleClass.SetInt(i);
        simpleClass.SetFloat(ranGen.Rndm()*10);
        nX=(Int_t)(ranGen.Rndm()*10);
        printf("%d\n",nX);
        tempVecDouble.clear();
        for(int j=0;j<nX;j++) 
        {
            tempVecDouble.push_back(ranGen.Gaus());
        }
        simpleClass.SetVecDouble(tempVecDouble);
        tree->Fill();
    }

    file->Write();
    file->Close();
}