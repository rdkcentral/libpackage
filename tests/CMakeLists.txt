cmake_minimum_required(VERSION 3.10)
project(project-name)

set(CMAKE_CXX_STANDARD 11)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

#add_subdirectory(googletest)
include(FetchContent)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)

FetchContent_MakeAvailable(googletest)

enable_testing()
find_package(SQLite3 REQUIRED)
include_directories(${SQLite3_INCLUDE_DIRS})
link_libraries(${SQLite3_LIBRARIES})

add_executable(PackageImplTest PackageImplTest.cpp)
target_compile_options(PackageImplTest PRIVATE -fpermissive)
target_link_libraries(PackageImplTest
        PRIVATE Package
        gtest gtest_main gmock pthread ${SQLite3_LIBRARIES})
install(TARGETS PackageImplTest DESTINATION bin)
#add_test(NAME PackageImplTest COMMAND PackageImplTest)
