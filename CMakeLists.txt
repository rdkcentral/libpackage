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
project(package)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_minimum_required(VERSION 3.22)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

option(ENABLE_RALF_SUPPORT "Enable RALF support" OFF)
option (BUILD_TEST_APP "Build test application" OFF)

set(LIBPACKAGE_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
if(ENABLE_RALF_SUPPORT)
message(STATUS "RALF support is enabled")
    add_subdirectory(src/ralf)
else()
    message(STATUS "Building without RALF support")
    include_directories(include/legacy)
    add_subdirectory(src/legacy)
endif()

if(BUILD_TEST_APP)
    add_subdirectory(app)
endif()

if(LIBPACKAGE_L1_TESTS)
    add_subdirectory(tests)
endif()
