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
find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(LibArchive REQUIRED)

include_directories(include)

add_library(Package SHARED ${PACKAGE_SRCS})

set_target_properties(Package PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(Package
    PRIVATE ${LibArchive_LIBRARIES}
    PRIVATE ${Boost_FILESYSTEM_LIBRARY}
    PRIVATE ${Boost_SYSTEM_LIBRARY}
    PRIVATE ${SQLITE_LIBRARIES}
)
install(TARGETS Package DESTINATION lib)
install(FILES
    include/PackageImpl.h
    include/IPackageImpl.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
add_subdirectory(app)
add_subdirectory(tests)
