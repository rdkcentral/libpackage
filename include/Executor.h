/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 Liberty Global Service B.V.
 * Copyright 2025 RDK Management
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

#include "Config.h"
#include "Debug.h"
#include "DataStorage.h"

#include <array>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace packagemanager
{

    namespace Filesystem
    {
        struct StorageDetails;
    }

    class Executor
    {

        public:
        uint32_t Configure(const std::string &configString);

        uint32_t Install(const std::string &type,
                         const std::string &id,
                         const std::string &version,
                         const std::string &url,
                         const std::string &appName,
                         const std::string &category);

        uint32_t Uninstall(const std::string &type,
                           const std::string &id,
                           const std::string &version,
                           const std::string &uninstallType);

        uint32_t GetStorageDetails(const std::string &type,
                                   const std::string &id,
                                   const std::string &version,
                                   Filesystem::StorageDetails &details);

        uint32_t GetAppDetailsList(const std::string &type,
                                   const std::string &id,
                                   const std::string &version,
                                   const std::string &appName,
                                   const std::string &category,
                                   std::vector<DataStorage::AppDetails> &appsDetailsList) const;

        uint32_t GetAppConfigPath(const std::string &path,
                                  std::string &appPath) const;
        uint32_t GetAppInstalledPath(const std::string &id,
                                               const std::string &version, std::string &appPath) const;
            uint32_t GetAppDetails(
                const std::string &id,
                DataStorage::AppDetails &appsDetailsList) const;

        uint32_t SetMetadata(const std::string &type,
                             const std::string &id,
                             const std::string &version,
                             const std::string &key,
                             const std::string &value);

        uint32_t ClearMetadata(const std::string &type,
                               const std::string &id,
                               const std::string &version,
                               const std::string &key);

        uint32_t GetMetadata(const std::string &type,
                             const std::string &id,
                             const std::string &version,
                             DataStorage::AppMetadata &metadata) const;

    private:
        void handleDirectories();
        void initializeDataBase(const std::string &dbpath);

        bool isAppInstalled(const std::string &type,
                            const std::string &id,
                            const std::string &version);

        void importAnnotations(const std::string &type,
                               const std::string &id,
                               const std::string &version,
                               const std::string &appPath);

        bool extract(std::string type,
                       std::string id,
                       std::string version,
                       std::string url,
                       std::string appName,
                       std::string category);

        void doUninstall(std::string type,
                         std::string id,
                         std::string version,
                         std::string uninstallType);

        void doMaintenance();

        std::unique_ptr<packagemanager::DataStorage> dataBase;

        using LockGuard = std::lock_guard<std::mutex>;

        std::mutex taskMutex{};
        typedef std::pair<std::string, std::string> app; // id, version
        std::vector<app> lockedApps;                     // id, version

        Config config{};
    };

} // namespace packagemanager