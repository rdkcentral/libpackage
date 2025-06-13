/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2025 RDK Management
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
 *
 *
 */
#pragma once

// @stubgen:skip

#include <string>
#include <memory>
#include <map>
#include <vector>
#include <set>

namespace packagemanager
{

    enum Result : uint8_t
    {
        SUCCESS,
        FAILED,
        VERSION_MISMATCH
    };

    typedef enum : uint8_t
    {
        UNKNOWN = 0,
        INTERACTIVE,
        SYSTEM
    } ApplicationType;

    struct ConfigMetaData
    {
        bool dial;
        bool wanLanAccess;
        bool thunder;
        int32_t systemMemoryLimit;
        int32_t gpuMemoryLimit;
        std::vector<std::string> envVars;
        uint32_t userId;
        uint32_t groupId;
        uint32_t dataImageSize;

        bool resourceManagerClientEnabled;
        std::string dialId;
        std::string command;
        ApplicationType appType;
        std::string appPath;
        std::string runtimePath;

        std::string logFilePath;
        uint32_t logFileMaxSize;
        std::string logLevels; // json array of strings
        bool mapi;
        std::set<std::string> fkpsFiles;

        std::string fireboltVersion;
        bool enableDebugger;
    };

    typedef std::pair<std::string, std::string> ConfigMetadataKey;
    typedef std::map<ConfigMetadataKey, ConfigMetaData> ConfigMetadataArray;

    typedef std::pair<std::string, std::string> NameValue;
    typedef std::vector<NameValue> NameValues;
    class IPackageImpl
    {
    public:
        virtual ~IPackageImpl() = default;

        virtual Result Initialize(const std::string &configStr, ConfigMetadataArray &aConfigMetadata) = 0;

        virtual Result Install(const std::string &packageId, const std::string &version, const NameValues &additionalMetadata, const std::string &fileLocator, ConfigMetaData &configMetadata) = 0;
        virtual Result Uninstall(const std::string &packageId) = 0;

        virtual Result Lock(const std::string &packageId, const std::string &version, std::string &unpackedPath, ConfigMetaData &configMetadata) = 0;
        virtual Result Unlock(const std::string &packageId, const std::string &version) = 0; // XXX: Not in LibPackage-interface doc
        //Fix for build error
        virtual Result Lock(const std::string &packageId, const std::string &version, std::string &unpackedPath, ConfigMetaData &configMetadata, NameValues &additionalLocks) { return SUCCESS; }

        static std::shared_ptr<packagemanager::IPackageImpl> instance();
    };

}
