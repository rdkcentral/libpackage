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

project(LibPackage)

set(LIBPACKAGE_SRCS
    src/LibPackage.cpp
)

include_directories(inc)

add_library(LibPackage SHARED
    ${LIBPACKAGE_SRCS}
)

set_target_properties(LibPackage PROPERTIES
    OUTPUT_NAME "Package"
    SUFFIX ".so"
)

install(TARGETS LibPackage DESTINATION lib)
install(FILES
    inc/LibPackage.h
    inc/IPackageImpl.h
    DESTINATION include/LibPackage
)
