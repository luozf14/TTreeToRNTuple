cmake_minimum_required(VERSION 3.14)
project(TTreeToRNTuple)

set(CMAKE_CXX_STANDARD 17)

# Find ROOT
find_package(ROOT REQUIRED ROOTNTuple)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ROOT_CXX_FLAGS} -O3")
include_directories(${ROOT_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR})
link_directories(${ROOT_LIBRARY_DIR})

add_executable(ConverterSimple src/ConverterSimple.cxx)
add_executable(Viewer src/Viewer.cxx)

target_link_libraries(ConverterSimple ${ROOT_LIBRARIES})
target_link_libraries(Viewer ${ROOT_LIBRARIES})

