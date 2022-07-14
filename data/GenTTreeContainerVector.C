void GenTTreeContainerVector()
{
    TFile *file = new TFile("TTreeContainerVector.root", "RECREATE");
    TTree *tree = new TTree("T3", "TTree with STL containers containing simple c++ types");

    std::vector<Float_t> vec_float;
    std::vector<bool> vec_bool;
    ROOT::RVec<double> RVec_double;
    ROOT::RVec<std::string> RVec_string;
    std::string string_1;

    tree->Branch("vec_float","std::vector<Float_t>",&vec_float);
    tree->Branch("vec_bool",&vec_bool);
    tree->Branch("RVec_double",&RVec_double);
    tree->Branch("RVec_string",&RVec_string);
    tree->Branch("string_1","std::string",&string_1);





    TRandom3 ranGen;
    Int_t nX;
    for(int i=0; i<20; i++)
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
        string_1 = std::to_string(i);
        
        tree->Fill();
    }

    file->Write();
    file->Close();
}