/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2025 LTTS Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 #pragma once

 #include <fstream>
 #include <iostream>
 #include <stdlib.h>
 #include <string>
 #include <json/json.h>
 
 typedef enum _ApplicationType
 {
    APPLICATION_TYPE_UNKNOWN = 0,
    APPLICATION_TYPE_INTERACTIVE,
    APPLICATION_TYPE_SYSTEM
 } ApplicationType;

 typedef struct _ConfObject
 {
    uint32_t systemMemoryLimit;
    uint32_t gpuMemoryLimit;
    bool dial;
    std::string dialId;
    bool wanLanAccess;
    bool thunder;
    std::string command;
    ApplicationType type;
    std::string appPath;
    std::string runtimePath;
    std::vector<std::string> envVariables;
    std::string fireboltVersion;
    uint32_t cores;
 } ConfObject;

uint32_t Lock(const std::string& packageId, const std::string& version, ConfObject& response);
uint32_t Install(const std::string& packageId, const std::string& version,const std::string& fileLocator, ConfObject& response);
uint32_t Initialize(ConfObject& response);
