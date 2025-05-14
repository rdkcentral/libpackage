/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 Liberty Global Service B.V.
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
        enum ReturnCodes
        {
            ERROR_NONE = 0, // Core::ERROR_NONE,
            ERROR_GENERAL = 1,
            ERROR_WRONG_PARAMS = 1001,
            ERROR_TOO_MANY_REQUESTS = 1002,
            ERROR_ALREADY_INSTALLED = 1003,
            ERROR_WRONG_HANDLE = 1007,
            ERROR_APP_LOCKED = 1009,
            ERROR_APP_UNINSTALLING = 1010
        };

        enum class OperationStatus
        {
            SUCCESS,
            FAILED,
            PROGRESS,
            CANCELLED
        };
        enum class OperationType
        {
            INSTALLING,
            UNINSTALLING
        };
        struct OperationStatusEvent
        {
            std::string handle, type, id, version, details;
            OperationType operation;
            OperationStatus status;
            OperationStatusEvent() {}
            OperationStatusEvent(const std::string &handle_, const packagemanager::Executor::OperationType &operation_, const std::string &type_, const std::string &id_,
                                 const std::string &version_, const packagemanager::Executor::OperationStatus &status_, const std::string &details_) : handle(handle_), type(type_), id(id_), version(version_), details(details_), operation(operation_), status(status_)
            {
            }
            static std::string statusStr(const packagemanager::Executor::OperationStatus &status)
            {
                switch (status)
                {
                case packagemanager::Executor::OperationStatus::SUCCESS:
                    return "Success";
                case packagemanager::Executor::OperationStatus::FAILED:
                    return "Failed";
                case packagemanager::Executor::OperationStatus::PROGRESS:
                    return "Progress";
                case packagemanager::Executor::OperationStatus::CANCELLED:
                    return "Cancelled";
                }
                return "";
            }
            std::string statusStr() const
            {
                return OperationStatusEvent::statusStr(status);
            }
            static std::string operationStr(const packagemanager::Executor::OperationType &operation)
            {
                switch (operation)
                {
                case packagemanager::Executor::OperationType::INSTALLING:
                    return "Installing";
                case packagemanager::Executor::OperationType::UNINSTALLING:
                    return "Uninstalling";
                }
                return "";
            }
            std::string operationStr() const
            {
                return OperationStatusEvent::operationStr(operation);
            }
        };

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
                           const std::string &uninstallType,
                           std::string &handle);

        uint32_t Lock(const std::string &type,
                      const std::string &id,
                      const std::string &version,
                      std::string &unLockedPath);

        uint32_t Unlock(const std::string &id,
                        const std::string &version);

        uint32_t GetLockInfo(const std::string &id, const std::string &version, std::string &unpackedPath, bool &locked);

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

        void doInstall(std::string type,
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