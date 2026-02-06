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


find_package(libralf REQUIRED)
set (PACKAGE_SRCS RalfPackageHandler.cpp)

option(ENABLE_LOCAL_MOUNT "Enable local mount support" OFF)
option(DISABLE_DEPENDENCY_CHECK "Disable dependency check" ON)
if(ENABLE_LOCAL_MOUNT)
    #print a message 
    message(STATUS "Local mount is enabled")    
    list(APPEND PACKAGE_SRCS LoopMount.cpp)
    add_definitions("-DENABLE_LOCAL_MOUNT")
else()
    message(STATUS "Local mount is disabled")
endif(ENABLE_LOCAL_MOUNT)

if(DISABLE_DEPENDENCY_CHECK)
    message(STATUS "Dependency check is disabled")
    add_definitions("-DDISABLE_DEPENDENCY_CHECK")
else()
    message(STATUS "Dependency check is enabled")
endif()

if(NOT RDK_PACKAGE_CERT_PATH STREQUAL "")
    message(STATUS "Using custom package certificate path: ${RDK_PACKAGE_CERT_PATH}")
    add_definitions("-DRDK_PACKAGE_CERT_PATH=\"${RDK_PACKAGE_CERT_PATH}\"")
else()
    message(STATUS "Using default package certificate path")
endif()

add_library(Package SHARED ${PACKAGE_SRCS})
set_target_properties(Package PROPERTIES VERSION 1.1.0 SOVERSION 1)


if(DAC_APP_PATH)
    target_compile_definitions(Package PRIVATE DAC_APP_PATH="${DAC_APP_PATH}")
endif()


target_link_libraries(Package PRIVATE ralf jsoncpp)
target_include_directories(Package PRIVATE  ${LIBPACKAGE_BASE_DIR}/include/ralf)

install(TARGETS Package DESTINATION lib)

install(FILES
        ${LIBPACKAGE_BASE_DIR}/include/ralf/RalfPackageImpl.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

