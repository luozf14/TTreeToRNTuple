# TTreeToRNTuple
A tool that takes as input a root file containing TTree and converts TTree to RNTuple.

## Features
- Supports TTree with branches of most-common types.
  - All basic C++ data types, e.g., ``int``, ``float``. 
  - 1D C++ array of fixed or variable length, e.g., ``int a[10]``, ``float b[n]``.
  - All STL containers that are supported by RNTuple: ``std::string``, ``std::array<T, N>``, ``std::vector<T> and ROOT::RVec<T>``, ``std::pair<T1, T2>``, ``std::tuple<T1, …, Tn>``.
  - Any user-defined class with the corresponding dictionary.
  - Nested types. Currently one needs to generate the dictionary manually for nested types in order to convert them.
- This tool can be used as a command-line tool or a C++ library. 

## Prerequisites
- ROOT with v7 enabled (master branch in https://github.com/root-project/root, not release versions!)
- C++17
- CMake>=3.16

## How to use - As a commandline tool
### Compile
Make a build directory and enter it.
```
$ mkdir build && cd build
```

cmake & make 
```
$ cmake ../ && make
```
### Run
To read the usage, simply run
```
$ ./GenericConverter -h
Usage: ./GenericConverter -i <input.root> -o <output.ntuple> -t(ree) <tree name> [-d(ictionary) <dictionary name>] [-s(ub branch) <branch name>][-c(ompression) <compression algorithm>] [-p(rint conversion progress)]
```
- The program takes at least three inputs: ``-i`` the input file, ``-o`` the output file, and ``-t`` the name of the TTree which is to be converted.

- If the TTree contains user-defined classes, one needs to specify the corresponding dictionaries following ``-d``.

- Option ``-s`` specifies the branches that need to be converted. If no ``-s`` is enabled, the tool will convert all branches in the input TTree.

- Option ``-c`` specifies the compression algorithm used when generating the RNTuple file. It can be ``zlib``, ``lz4``, ``lzma``, ``zstd``, or ``none``. If no ``-c`` is enabled, no compression will be used.

- Option ``-p`` enables printing the conversion progress. 

### Example
```
$ ./GenericConverter -i ../data/TTreeMixed.root -o out.ntuple -t MixedTree -d ../data/SimpleClass_cxx.so -c lzma -p
Load dictionary '../data/SimpleClass_cxx.so' successfully!
Number of entries in tree 'MixedTree': 2000.
In input file '../data/TTreeMixed.root' detect leaf name: simpleClass; leaf type: SimpleClass; leaf title: simpleClass; leaf length: 1; leaf type size: 0
In input file '../data/TTreeMixed.root' detect leaf name: x; leaf type: Float_t; leaf title: x[3]; leaf length: 3; leaf type size: 4
In input file '../data/TTreeMixed.root' detect leaf name: y; leaf type: Double_t; leaf title: y[5]; leaf length: 5; leaf type size: 8
In input file '../data/TTreeMixed.root' detect leaf name: nZ; leaf type: Int_t; leaf title: nZ; leaf length: 1; leaf type size: 4
In input file '../data/TTreeMixed.root' detect leaf name: z; leaf type: Double_t; leaf title: z[nZ]; leaf length: 1; leaf type size: 8
In input file '../data/TTreeMixed.root' detect leaf name: array_float; leaf type: Float_t; leaf title: array_float[10]; leaf length: 10; leaf type size: 4
In input file '../data/TTreeMixed.root' detect leaf name: vec_float; leaf type: vector<float>; leaf title: vec_float; leaf length: 1; leaf type size: 0
In input file '../data/TTreeMixed.root' detect leaf name: vec_bool; leaf type: vector<bool>; leaf title: vec_bool; leaf length: 1; leaf type size: 0
In input file '../data/TTreeMixed.root' detect leaf name: RVec_double; leaf type: ROOT::VecOps::RVec<double>; leaf title: RVec_double; leaf length: 1; leaf type size: 0
In input file '../data/TTreeMixed.root' detect leaf name: RVec_string; leaf type: ROOT::VecOps::RVec<string>; leaf title: RVec_string; leaf length: 1; leaf type size: 0
In input file '../data/TTreeMixed.root' detect leaf name: pair_; leaf type: pair<int,float>; leaf title: pair_; leaf length: 1; leaf type size: 0
In input file '../data/TTreeMixed.root' detect leaf name: tuple_; leaf type: tuple<string,int,float>; leaf title: tuple_; leaf length: 1; leaf type size: 0
In input file '../data/TTreeMixed.root' detect leaf name: string_; leaf type: string; leaf title: string_; leaf length: 1; leaf type size: 0
Add field: x; field type name: std::array<float,3>
Add field: y; field type name: std::array<double,5>
Add field: nZ; field type name: std::int32_t
Add field: z; field type name: std::vector<double>
Add field: array_float; field type name: std::array<float,10>
Add field: simpleClass; field type name: SimpleClass
Add field: vec_float; field type name: std::vector<float>
Add field: vec_bool; field type name: std::vector<bool>
Add field: RVec_double; field type name: ROOT::VecOps::RVec<double>
Add field: RVec_string; field type name: ROOT::VecOps::RVec<std::string>
Add field: pair_; field type name: std::pair<std::int32_t,float>
Add field: tuple_; field type name: std::tuple<std::string,std::int32_t,float>
Add field: string_; field type name: std::string
Warning in <[ROOT.NTuple] Warning /home/luozf/Documents/root/tree/ntuple/v7/src/RPageStorageFile.cxx:51 in ROOT::Experimental::Detail::RPageSinkFile::RPageSinkFile(std::string_view, const ROOT::Experimental::RNTupleWriteOptions&)>: The RNTuple file format will change. Do not store real data with this version of RNTuple!
Processing entry 2000 of 2000 [100.0% completed]
```

## How to use - As a C++ library
``Example01.cxx`` in the project source directory shows an example of using this tool as a C++ library. 
- The constructor takes at least three inputs: input file, output file, and the TTree name. 
- Compression algorithm (``zlib``, ``lz4``, ``lzma``, ``zstd``, or ``none``) and level (from ``0`` to ``9``) can be set by ``SetCompressionAlgoLevel(std::string compressionAlgo, int compressionLevel)``. One can also use ``SetCompressionAlgo(std::string compressionAlgo)`` without specifying compression level. By default, the library does not use any compression.
- If the input TTree contains branches of user-defined classes, one has to specify the dictionaries of those classes by ``SetDictionary(std::vector<std::string> dictionary)``.
- By default all branches in the input TTree will be converted. If only some of them need to be converted, one needs to select these branches by ``SelectBranches(std::vector<std::string> subBranches)``.
- The library provides an interface to set the callback function of printing conversion progress. By default no progress will be printed. User can setup self-defined lambda function by ``SetUserProgressCallbackFunc([](int current, int total){/***/})``, or use the callback function provided by the library ``SetDefaultProgressCallbackFunc()``. For more details, see ``Example01.cxx``.
- Upon setting up the required and optional parameters, the conversion is proceeded by calling ``Convert()``.

## Test
### Unit test
The unit test is under directory ``test/``. For TTree containing branches of all supported types except ``RVec<T>``, at least up to 1e8 entries, the conversion works well, and all data can be migrated correctly. When TTree contains branches of ``RVec<T>``, the number of entries should not exceed 1e5, other wise the conversion will crash. We are still working on this issue. Please do not use this library/command-line tool with ``RVec<T>``. 
### Test data sets
This library has been tested with the data that can be downloaded from https://root.cern/files/RNTuple/treeref/. 







