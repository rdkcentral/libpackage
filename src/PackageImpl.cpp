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
 */

#include "PackageImpl.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace packagemanager
{
    bool getKeyValue(const NameValues &additionalMetadata, const std::string &key, std::string &value)
    {
        for (const auto &pair : additionalMetadata)
        {
            if (pair.first == key)
            {
                value = pair.second;
                return true;
            }
        }
        return false;
    }

    Result PackageImpl::Initialize(const std::string &configString, ConfigMetadataArray &configMetadata)
    {

        // Initialize the executor
        uint32_t result = executor.Configure(configString); // Assuming empty config for now, can be replaced with actual config string
        INFO("PackageImpl initialized, Status : ", result);
        return result == Executor::ReturnCodes::ERROR_NONE ? SUCCESS : FAILED;
    }

    Result PackageImpl::Install(const std::string &packageId, const std::string &version, const NameValues &additionalMetadata, const std::string &fileLocator, ConfigMetaData &configMetadata)
    {
        std::string type, category, appName;
        // Extract additional metadata
        getKeyValue(additionalMetadata, "type", type);
        getKeyValue(additionalMetadata, "category", category);
        getKeyValue(additionalMetadata, "appName", appName);

        uint32_t result = executor.Install(type, packageId, version, fileLocator, appName, category);
        // The executor will handle the installation process, so we return SUCCESS here
        return result == Executor::ReturnCodes::ERROR_NONE ? SUCCESS : FAILED;
    }

    Result PackageImpl::Uninstall(const std::string &packageId)
    {
	if (packageId.empty()) {
            ERROR("Package ID is empty. Cannot proceed with uninstallation.");
            return FAILED;
        }
        std::string uninstallType = "full"; // Assuming full uninstall
        std::string handle;
        DataStorage::AppDetails appDetails;
        INFO("Retrieving app details for packageId: ", packageId);
        executor.GetAppDetails(packageId, appDetails);
        // INFO("Uninstalling app: {", appDetails.type,", ", appDetails.version, ", ", appDetails.appName, "}");
        uint32_t result = executor.Uninstall(appDetails.type, packageId, appDetails.version, uninstallType, handle);
        INFO("Uninstall handle: ", handle);
        return result == Executor::ReturnCodes::ERROR_NONE ? SUCCESS : FAILED;
    }
    Result PackageImpl::GetList(std::string &packageList)
    {
        Result result = FAILED;
        std::vector<packagemanager::DataStorage::AppDetails> appsDetailsList{};
        auto rc = executor.GetAppDetailsList("", "", "", "", "", appsDetailsList);
        if (rc == Executor::ReturnCodes::ERROR_NONE)
        {
            INFO("[PackageImpl::GetList] Retrieved ", appsDetailsList.size(), " app details");
            boost::property_tree::ptree packages;

            for (const auto &app : appsDetailsList)
            {
                INFO("App Details: ", app.type, ", ", app.id, ", ", app.version, ", ", app.appName, ", ", app.category, ", ", app.url);
                // Create a JSON object for each app and add it to the packages array
                boost::property_tree::ptree package;
                package.put("packageId", app.id);
                package.put("version", app.version);
                packages.push_back(std::make_pair("", package));
            }
            INFO("Total packages found: ", packages.size());
            // Write the property tree to a JSON string
            std::ostringstream jsonStream;
            boost::property_tree::write_json(jsonStream, packages);
            packageList = jsonStream.str();
            result = SUCCESS;
        }
        else
        {
            ERROR("Failed to retrieve package list, error code: ", rc);
        }
        return result;
    }

    Result PackageImpl::Lock(const std::string &packageId, const std::string &version, std::string &unpackedPath, ConfigMetaData &configMetadata)
    {
        DataStorage::AppDetails appDetails;
        INFO("Retrieving app details for packageId: ", packageId);
        if (!executor.GetAppDetails(packageId, appDetails) == Executor::ReturnCodes::ERROR_NONE)
        {
            ERROR("Failed to retrieve app details for packageId: ", packageId);
            return FAILED;
        }
        INFO("Locking app: {", appDetails.type, ", ", appDetails.version, ", ", appDetails.appName, "}");
        auto result = executor.Lock(appDetails.type, packageId, appDetails.version, unpackedPath);
        // The executor will handle the locking process, so we return SUCCESS here
        INFO("Package ", packageId, " version ", appDetails.version, " locked successfully.", " Handle: ", unpackedPath);
        return result == Executor::ReturnCodes::ERROR_NONE ? SUCCESS : FAILED;
    }

    Result PackageImpl::Unlock(const std::string &packageId, const std::string &version)
    {
	if (packageId.empty() || version.empty()) {
            ERROR("Package ID or version is empty. Cannot proceed with unlocking.");
            return FAILED;
        }
        std::string handle;
        auto result = executor.Unlock(packageId, version);
        // The executor will handle the unlocking process, so we return SUCCESS here
        return result == Executor::ReturnCodes::ERROR_NONE ? SUCCESS : FAILED;
    }

    Result PackageImpl::GetLockInfo(const std::string &packageId, const std::string &version, std::string &unpackedPath, bool &locked)
    {

        return SUCCESS;
    }
    std::shared_ptr<packagemanager::IPackageImpl> IPackageImpl::instance()
    {

        std::shared_ptr<packagemanager::IPackageImpl> packageImpl =
            std::make_shared<packagemanager::PackageImpl>();

        return packageImpl;
    }
}
