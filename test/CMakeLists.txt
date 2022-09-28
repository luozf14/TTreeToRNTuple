# Find GTest
find_package(GTest REQUIRED)

add_library(SimpleClass SHARED ${CMAKE_CURRENT_SOURCE_DIR}/SimpleClass.cxx)
target_include_directories(SimpleClass PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})

enable_testing()
add_executable(unittest01 unittest01.cxx)
target_include_directories(unittest01  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(unittest01 PRIVATE GTest::GTest GTest::Main TTreeToRNTuple SimpleClass ${ROOT_LIBRARIES})
gtest_discover_tests(unittest01)