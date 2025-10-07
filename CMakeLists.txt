# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2025 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

project(Package)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
set(PACKAGE_SRCS
    src/PackageImpl.cpp
    src/SqlDataStorage.cpp
    src/Filesystem.cpp
    src/File.cpp
    src/Archives.cpp
    src/Executor.cpp
    src/Config.cpp
)
find_package(Sqlite REQUIRED)
find_package(Boost COMPONENTS filesystem REQUIRED)

#This is set only for development in apple 
#set(LibArchive_INCLUDE_DIR "/opt/homebrew/opt/libarchive/include")
find_package(LibArchive REQUIRED)

include_directories(include)
include(GNUInstallDirs)

add_library(Package SHARED ${PACKAGE_SRCS})

set_target_properties(Package PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_include_directories(Package
    PRIVATE ${LibArchive_INCLUDE_DIR}
    PRIVATE ${Boost_INCLUDE_DIRS}
    PRIVATE ${SQLITE_INCLUDE_DIRS}
)

target_link_libraries(Package
    PRIVATE ${LibArchive_LIBRARIES}
    PRIVATE ${Boost_FILESYSTEM_LIBRARY}
    PRIVATE ${Boost_SYSTEM_LIBRARY}
    PRIVATE ${SQLITE_LIBRARIES}
)
install(TARGETS Package DESTINATION lib)
install(FILES
    include/PackageImpl.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
add_subdirectory(app)
if(LIBPACKAGE_L1_TESTS)
    add_subdirectory(tests)
endif()
