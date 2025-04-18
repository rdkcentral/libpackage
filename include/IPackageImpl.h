/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
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

// @stubgen:skip

#include <string>
#include <memory>
#include <map>
#include <vector>

namespace packagemanager {

enum Result : uint8_t {
    SUCCESS,
    FAILED
};

struct ConfigMetaData {
    bool dial;
    bool wanLanAccess;
    bool thunder;
    int32_t systemMemoryLimit;
    int32_t gpuMemoryLimit;
    std::vector<std::string> envVars;
    uint32_t userId;
    uint32_t groupId;
    uint32_t dataImageSize;
};

typedef std::pair<std::string, std::string> ConfigMetadataKey;
typedef std::map<ConfigMetadataKey, ConfigMetaData>  ConfigMetadataArray;

class IPackageImpl {
    public:
    virtual ~IPackageImpl() = default;

    virtual Result Initialize(ConfigMetadataArray &configMetadata) = 0;

    virtual Result Install(const std::string &packageId, const std::string &version, const std::string &fileLocator, ConfigMetaData &configMetadata) = 0;
    virtual Result Uninstall(const std::string &packageId) = 0;

    virtual Result GetList(std::string &packageList) = 0;

    virtual Result Lock(const std::string &packageId, const std::string &version, std::string &unpackedPath, ConfigMetaData &configMetadata) = 0;
    virtual Result Unlock(const std::string &packageId, const std::string &version) = 0;

    virtual Result GetLockInfo(const std::string &packageId, const std::string &version, std::string &unpackedPath, bool &locked) = 0;

    static std::shared_ptr<packagemanager::IPackageImpl> instance();

};

}
