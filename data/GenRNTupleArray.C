#include <ROOT/RField.hxx>
#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleOptions.hxx>

using RCollectionNTupleWriter = ROOT::Experimental::RCollectionNTupleWriter;
using REntry = ROOT::Experimental::REntry;
using RFieldBase = ROOT::Experimental::Detail::RFieldBase;
using RNTupleModel = ROOT::Experimental::RNTupleModel;
using RNTupleWriteOptions = ROOT::Experimental::RNTupleWriteOptions;
using RNTupleWriter = ROOT::Experimental::RNTupleWriter;

void GenRNTupleArray()
{
    auto model = RNTupleModel::Create();

    auto fldAx = model->MakeField<std::vector<float>>("Array_x");
    auto ntuple = RNTupleWriter::Recreate(std::move(model), "F", "out.tuple");
    for(int i=0;i<100;i++)
    {
        fldAx->clear();
        for(int j=0;j<10;j++)
        {
            fldAx->push_back(j*100.);
        }
    ntuple->Fill();

    }

    
}