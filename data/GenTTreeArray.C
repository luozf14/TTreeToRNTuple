void GenTTreeArray()
{
    TFile *file = new TFile("TTreeArray.root", "RECREATE");
    TTree *tree = new TTree("T2", "TTree with c++ arrays");

    Float_t x[3];
    Double_t y[5];
    Int_t nZ;
    Double_t z[10];

    tree->Branch("x", x, "x[3]/F");
    tree->Branch("y", y, "y[5]/D");
    tree->Branch("nZ", &nZ, "nZ/I");
    tree->Branch("z", z, "z[nZ]/D");

    TRandom3 ranGen;
    for(int i=0; i<10; i++)
    {
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
        tree->Fill();
    }

    file->Write();
    file->Close();
}