cmake_minimum_required(VERSION 3.10)
project(project-name)

set(CMAKE_CXX_STANDARD 11)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_subdirectory(googletest)

enable_testing()

add_executable(PackageImplTest tests/PackageImplTest.cpp)
target_link_libraries(PackageImplTest gtest gtest_main)
add_test(NAME PackageImplTest COMMAND PackageImplTest)