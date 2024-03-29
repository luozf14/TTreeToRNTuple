cmake_minimum_required(VERSION 3.16)
project(TTreeToRNTuple)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
SET(CMAKE_BUILD_TYPE "Release")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -Wall")

# Find ROOT
find_package(ROOT CONFIG REQUIRED ROOTNTuple)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ROOT_CXX_FLAGS}")
include_directories(${ROOT_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR})
link_directories(${ROOT_LIBRARY_DIR})

# libTTreeToRNTuple
add_library (TTreeToRNTuple SHARED src/TTreeToRNTuple.cxx)
target_include_directories(TTreeToRNTuple PUBLIC ${PROJECT_SOURCE_DIR}/include )

# Viewer for inspect certain entry of converted files
add_executable(ViewerUserDefClass src/ViewerUserDefClass.cxx)
target_link_libraries(ViewerUserDefClass ${ROOT_LIBRARIES})
add_executable(Viewer src/Viewer.cxx)
target_link_libraries(Viewer ${ROOT_LIBRARIES})

# used as a C++ library
add_executable(example01 Example01.cxx)
target_include_directories(example01 PRIVATE  ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(example01 PRIVATE TTreeToRNTuple ${ROOT_LIBRARIES})

# used as a commandline tool
add_executable(GenericConverter src/GenericConverter.cxx)
target_include_directories(GenericConverter PRIVATE  ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(GenericConverter PRIVATE TTreeToRNTuple ${ROOT_LIBRARIES})

# unit test
add_subdirectory(test)