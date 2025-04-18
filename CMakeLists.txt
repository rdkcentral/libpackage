# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2020 RDK Management
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

set(PACKAGE_SRCS
    src/LibPackage.cpp
)

include_directories(include)


add_library(Package SHARED
    ${PACKAGE_SRCS}
)
set_target_properties(Package PROPERTIES VERSION 1.0.0 SOVERSION 1)

install(TARGETS Package DESTINATION lib)
install(FILES
    include/LibPackage.h
    include/IPackageImpl.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
