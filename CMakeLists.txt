cmake_minimum_required(VERSION 3.14)
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

add_executable(ConverterSTLContainer src/ConverterSTLContainer.cxx)
add_executable(ConverterSimple src/ConverterSimple.cxx)
add_executable(ConverterArray src/ConverterArray.cxx)
add_executable(gen_physlite src/gen_physlite.cxx)
add_executable(Viewer src/Viewer.cxx)

target_link_libraries(ConverterSTLContainer ${ROOT_LIBRARIES})
target_link_libraries(ConverterSimple ${ROOT_LIBRARIES})
target_link_libraries(ConverterArray ${ROOT_LIBRARIES})
target_link_libraries(gen_physlite ${ROOT_LIBRARIES})

target_link_libraries(Viewer ${ROOT_LIBRARIES})

