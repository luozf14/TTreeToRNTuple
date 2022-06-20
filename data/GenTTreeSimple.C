void GenTTreeSimple()
{
    TFile *file = new TFile("TTreeSimple.root", "RECREATE");
    TTree *tree = new TTree("T1", "TTree with simple variable");

    Float_t x,y,z;
    Double_t d;
    Int_t intg;

    tree->Branch("x", &x, "x/F");
    tree->Branch("y", &y, "y/F");
    tree->Branch("z", &z, "z/F");
    tree->Branch("d", &d, "d/D");
    tree->Branch("intg", &intg, "intg/I");

    TRandom3 R;
    for(int i=0; i<1000; i++)
    {
        x=R.Rndm();
        y=R.Rndm();
        z=R.Gaus(0.5,1);
        d=x*x+y*y;
        intg=i;
        tree->Fill();
    }

    file->Write();
    file->Close();
}