# TTreeToRNTuple
A tool that takes as input a root file containing TTree and converts TTree to RNTuple.

## Features
- Supports TTree with branches of most-common types.
  - All basic C++ data types, e.g., ``int``, ``float``. 
  - 1D C++ array of fixed or variable length, e.g., ``int a[10]``, ``float b[n]``.
  - All STL containers that are supported by RNTuple: ``std::string``, ``std::array<T, N>``, ``std::vector<T> and ROOT::RVec<T>``, ``std::pair<T1, T2>``, ``std::tuple<T1, …, Tn>``.
  - Any user-defined class with the corresponding dictionary.
  - (TO DO) Nested types. 
- This tool can be used as a command-line tool or a C++ library. 

## Prerequisites
- ROOT with v7 enabled
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
Usage: ./GenericConverter -i <input.root> -o <output.ntuple> [-t(ree) <tree name>] [-d(ictionary) <dictionary names>] [-c(ompression) <compression algorithm>] [-m(ultithread)] 
```
The program takes at least two inputs: the root file containing the TTree, and the output file. 

If no ``-t`` option is specified, the program will convert the first TTree in the input root file.

If the TTree contains user-defined classes, one needs to specify the corresponding dictionaries following ``-d``.

Option ``-c`` specifies the compression algorithm used when generating the RNTuple file. It can be ``zlib``, ``lz4``, ``lzma``, ``zstd``, or ``none``. 

Option ``-m`` enables multithread.
### Example
```
$ ./GenericConverter -i ../data/TTreeMixed.root -o out.ntuple -t MixedTree -d ../data/SimpleClass_cxx.so -c zstd -m
dictionary=../data/SimpleClass_cxx.so
leaf name: simpleClass; leaf type: SimpleClass; leaf title: simpleClass; leaf length: 1; leaf type size: 0
leaf name: x; leaf type: Float_t; leaf title: x[3]; leaf length: 3; leaf type size: 4
leaf name: y; leaf type: Double_t; leaf title: y[5]; leaf length: 5; leaf type size: 8
leaf name: nZ; leaf type: Int_t; leaf title: nZ; leaf length: 1; leaf type size: 4
leaf name: z; leaf type: Double_t; leaf title: z[nZ]; leaf length: 1; leaf type size: 8
leaf name: vec_float; leaf type: vector<float>; leaf title: vec_float; leaf length: 1; leaf type size: 0
leaf name: vec_bool; leaf type: vector<bool>; leaf title: vec_bool; leaf length: 1; leaf type size: 0
leaf name: RVec_double; leaf type: ROOT::VecOps::RVec<double>; leaf title: RVec_double; leaf length: 1; leaf type size: 0
leaf name: RVec_string; leaf type: ROOT::VecOps::RVec<string>; leaf title: RVec_string; leaf length: 1; leaf type size: 0
leaf name: pair_; leaf type: pair<int,float>; leaf title: pair_; leaf length: 1; leaf type size: 0
leaf name: tuple_; leaf type: tuple<string,int,float>; leaf title: tuple_; leaf length: 1; leaf type size: 0
leaf name: string_; leaf type: string; leaf title: string_; leaf length: 1; leaf type size: 0
Add field: x; field type name: std::array<float,3>
Add field: y; field type name: std::array<double,5>
Add field: nZ; field type name: std::int32_t
Add field: z; field type name: std::vector<double>
Add field: simpleClass; field type name: SimpleClass
Add field: vec_float; field type name: std::vector<float>
Add field: vec_bool; field type name: std::vector<bool>
Add field: RVec_double; field type name: ROOT::VecOps::RVec<double>
Add field: RVec_string; field type name: ROOT::VecOps::RVec<std::string>
Add field: pair_; field type name: std::pair<std::int32_t,float>
Add field: tuple_; field type name: std::tuple<std::string,std::int32_t,float>
Add field: string_; field type name: std::string
Warning in <[ROOT.NTuple] Warning <path to root>/tree/ntuple/v7/src/RPageStorageFile.cxx:51 in ROOT::Experimental::Detail::RPageSinkFile::RPageSinkFile(std::string_view, const ROOT::Experimental::RNTupleWriteOptions&)>: The RNTuple file format will change. Do not store real data with this version of RNTuple!
Conversion completed!
```

## How to use - As a C++ library
``Example01.cxx`` in the project source directory shows an example of using this tool as a C++ library. The constructor takes at least three inputs: input root file, output RNTuple file, and the TTree name. One can also set up the compression algorithm, dictionaries, and enable multithread by ``TTreeToRNTuple::SetCompressionAlgo()``, ``TTreeToRNTuple::SetDictionary()`` and ``TTreeToRNTuple::EnableMultiThread()``, respectively. Upon setting up the required and optional parameters, the conversion is proceeded by calling ``TTreeToRNTuple::Convert()``. 

## To do
Currently nested types like ``std::vector<std::vector<double> >`` are not supported. However, if you generate the dictionaries for these nested types manually and load them into ROOT, you can still use this tool to convert TTree containing branches of nested types. 

We will add native support for nested types soon. 






