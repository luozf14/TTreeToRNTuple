cmake_minimum_required(VERSION 3.16)
project(TTreeToRNTuple)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -Wall")


# Find ROOT
find_package(ROOT CONFIG REQUIRED ROOTNTuple)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ROOT_CXX_FLAGS}")
include_directories(${ROOT_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR})
link_directories(${ROOT_LIBRARY_DIR})

add_executable(ConverterUserDefClass src/ConverterUserDefClass.cxx)
add_executable(ConverterSTLContainer src/ConverterSTLContainer.cxx)
add_executable(ConverterSimple src/ConverterSimple.cxx)
add_executable(ConverterArray src/ConverterArray.cxx)
# add_executable(gen_physlite src/gen_physlite.cxx)
add_executable(ViewerUserDefClass src/ViewerUserDefClass.cxx)
add_executable(Viewer src/Viewer.cxx)

target_link_libraries(ConverterUserDefClass ${ROOT_LIBRARIES})
target_link_libraries(ConverterSTLContainer ${ROOT_LIBRARIES})
target_link_libraries(ConverterSimple ${ROOT_LIBRARIES})
target_link_libraries(ConverterArray ${ROOT_LIBRARIES})
# target_link_libraries(gen_physlite ${ROOT_LIBRARIES})
target_link_libraries(ViewerUserDefClass ${ROOT_LIBRARIES})
target_link_libraries(Viewer ${ROOT_LIBRARIES})

add_library (TTreeToRNTuple SHARED src/TTreeToRNTuple.cxx)
target_include_directories(TTreeToRNTuple PUBLIC ${PROJECT_SOURCE_DIR}/include)

# used as a C++ library
add_executable(example01 Example01.cxx)
target_include_directories(example01 PRIVATE  ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(example01 PRIVATE TTreeToRNTuple ${ROOT_LIBRARIES})

# used as a commandline tool
add_executable(GenericConverter src/GenericConverter.cxx)
target_include_directories(GenericConverter PRIVATE  ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(GenericConverter PRIVATE TTreeToRNTuple ${ROOT_LIBRARIES})