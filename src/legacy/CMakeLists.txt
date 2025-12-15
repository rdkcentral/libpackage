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


list(APPEND CMAKE_MODULE_PATH "${LIBPACKAGE_BASE_DIR}/cmake/")
set(PACKAGE_SRCS
    PackageImpl.cpp
    SqlDataStorage.cpp
    Filesystem.cpp
    File.cpp
    Archives.cpp
    Executor.cpp
    Config.cpp
)
find_package(Sqlite REQUIRED)
find_package(Boost COMPONENTS filesystem REQUIRED)

#This is set only for development in apple 
#set(LibArchive_INCLUDE_DIR "/opt/homebrew/opt/libarchive/include")
find_package(LibArchive REQUIRED)

include_directories(include)
include(GNUInstallDirs)

add_library(libpackage SHARED ${PACKAGE_SRCS})

set_target_properties(libpackage PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_include_directories(libpackage
    PRIVATE ${LibArchive_INCLUDE_DIR}
    PRIVATE ${Boost_INCLUDE_DIRS}
    PRIVATE ${SQLITE_INCLUDE_DIRS}
)

target_link_libraries(libpackage
    PRIVATE ${LibArchive_LIBRARIES}
    PRIVATE ${Boost_FILESYSTEM_LIBRARY}
    PRIVATE ${Boost_SYSTEM_LIBRARY}
    PRIVATE ${SQLITE_LIBRARIES}
)
install(TARGETS libpackage DESTINATION lib)
install(FILES
    ${LIBPACKAGE_BASE_DIR}/include/legacy/PackageImpl.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
