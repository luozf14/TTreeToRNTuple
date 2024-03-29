#ifndef TTREETORNTUPLE_H
#define TTREETORNTUPLE_H

#include <ROOT/RField.hxx>
#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleOptions.hxx>

#include <Compression.h>
#include <TBranch.h>
#include <TFile.h>
#include <TLeaf.h>
#include <TROOT.h>
#include <TTree.h>
#include <TBranchElement.h>
#include <TBranchSTL.h>
#include <TClass.h>
#include <TSystem.h>
#include <TInterpreter.h>

#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <utility>

using RCollectionNTupleWriter = ROOT::Experimental::RCollectionNTupleWriter;
using REntry = ROOT::Experimental::REntry;
using RFieldBase = ROOT::Experimental::Detail::RFieldBase;
using RNTupleModel = ROOT::Experimental::RNTupleModel;
using RNTupleWriteOptions = ROOT::Experimental::RNTupleWriteOptions;
using RNTupleWriter = ROOT::Experimental::RNTupleWriter;
using RCompressionSetting = ROOT::RCompressionSetting;

typedef void (*callback_t)(int, int);

struct FlatField
{
    std::string treeName;
    std::string ntupleName;
    std::string typeName;
    Int_t leafTypeSize; // sizeof(leafType)
    Bool_t isVariableSizedArray;
    Int_t arrayLength; // 1 if non-array; size of the array if fixed-length array; maximun size if variable-sized array.
    std::unique_ptr<unsigned char[]> treeBuffer;
    std::unique_ptr<unsigned char[]> ntupleBuffer;
};

struct ContainerField
{
    std::string treeName;
    std::string ntupleName;
    std::string typeName;
    std::shared_ptr<void *> treeBuffer;
    std::unique_ptr<unsigned char[]> ntupleBuffer;
};

class TTreeToRNTuple
{
public:
    TTreeToRNTuple(std::string input, std::string output, std::string treeName);
    TTreeToRNTuple(std::string input, std::string output, std::string treeName, std::string compressionAlgo, int compressionLevel);
    TTreeToRNTuple(std::string input, std::string output, std::string treeName, std::string compressionAlgo, int compressionLevel, std::vector<std::string> dictionary);

    ~TTreeToRNTuple(){};

    void SetInputFile(std::string input);
    void SetOutputFile(std::string output);
    void SetTreeName(std::string treeName);
    void SetCompressionAlgo(std::string compressionAlgo);
    void SetCompressionAlgoLevel(std::string compressionAlgo, int compressionLevel);
    void SetDictionary(std::vector<std::string> dictionary);
    void SelectBranches(std::vector<std::string> subBranch);
    void SelectAllBranches();
    void SetUserProgressCallbackFunc(callback_t);

    std::string GetInputFile() { return fInputFile; };
    std::string GetOutputFile() { return fOutputFile; };
    std::string GetTreeName() { return fTreeName; };
    std::vector<std::string> GetDictionary() { return fDictionary; };

    void Convert();

private:
    RNTupleWriteOptions fWriteOptions;
    std::string fInputFile;
    std::string fOutputFile;
    std::string fTreeName;
    std::vector<std::string> fDictionary;
    std::vector<std::string> fSelectedBranches;
    std::vector<FlatField> fFlatFields;
    std::vector<ContainerField> fContainerFields;
    std::string SanitizeBranchName(std::string name);
    callback_t fCallbackFunc;
};
#endif // TTREETORNTUPLE_H
