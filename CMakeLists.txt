cmake_minimum_required(VERSION 2.8)
project(crdts)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(GTEST_INCLUDE /usr/local/include)
include_directories(${GTEST_INCLUDE})

set(TEST_SOURCE_FILES
        sb/timestamp.hh
        sb/timestamp.cc
        sb/lwwregister.hh
        sb/lwwregister.cc
        sb/orset.hh
        sb/orset.cc
        test/timestamp_unittest.cc
        test/lwwregister_uinttest.cc
        test/orset_uinttest.cc
)

find_library(GTEST_LIB NAMES libgtest.a PATHS /usr/local/lib)
find_library(GTEST_MAIN_LIB NAMES libgtest_main.a PATHS /usr/local/lib)

add_executable(crdts_test ${TEST_SOURCE_FILES})
target_link_libraries(crdts_test ${GTEST_LIB})
target_link_libraries(crdts_test ${GTEST_MAIN_LIB})
