void GenTTreeContainerSimple()
{
    TFile *file = new TFile("TTreeContainerSimple.root", "RECREATE");
    TTree *tree = new TTree("T3", "TTree with STL containers containing simple c++ types");

    std::vector<Double_t> vecX;
    // std::array<Float_t, 10> arrY;

    tree->Branch("vecX","std::vector<Double_t>",&vecX);
    // tree->Branch("arrY","std::array<Float_t, 10>",&arrY);


    TRandom3 ranGen;
    Int_t nX;
    for(int i=0; i<1000; i++)
    {
        vecX.clear();
        nX=(Int_t)(ranGen.Rndm()*100);
        for(int j=0;j<nX;j++) vecX.push_back(ranGen.Gaus(1,1));
        // for(int k=0;k<10;k++) arrY[k]=(Float_t)(ranGen.Rndm()*10);

        tree->Fill();
    }

    file->Write();
    file->Close();
}