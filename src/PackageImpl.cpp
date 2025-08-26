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

    Result PackageImpl::Initialize(const std::string &configString, ConfigMetadataArray &appMetaMap)
    {

        // Initialize the executor
        uint32_t result = executor.Configure(configString); // Assuming empty config for now, can be replaced with actual config string
        INFO("PackageImpl  initialized, Status : ", result);
        std::vector<DataStorage::AppDetails> appsDetailsList;
        const std::string type = "";
        const std::string id = "";
        const std::string version = "";
        const std::string appName = "";
        const std::string category = "";

        result = executor.GetAppDetailsList(type, id, version, appName, category, appsDetailsList);
        if (result != RETURN_SUCCESS)
        {
            ERROR("Failed to retrieve app details list, Status : ", result);
            return FAILED;
        }
        INFO("[PackageImpl::Initialize] Retrieved ", appsDetailsList.size(), " apps.");
        for (auto details : appsDetailsList)
        {
            ConfigMetaData configMetaData;
            std::string path, configPath;
            if (executor.GetAppInstalledPath(details.id, details.version, path) == SUCCESS)
            {
                configMetaData.appPath = path;
            }
            executor.GetAppConfigPath(path, configPath);
            INFO(details.appName, " configuration : ", configPath);

            if (populateConfigValues(configPath, configMetaData))
            {
                configMetaData.appType = INTERACTIVE;
                configMetaData.wanLanAccess = true;
                configMetaData.thunder = true;
                ConfigMetadataKey app = {details.id, details.version};
                appMetaMap[app] = configMetaData;
                INFO("Config metadata populated for app: ", details.appName);
            }
            else
            {
                ERROR("Failed to populate config metadata for app: ", details.appName);
            }
        }
        return result == RETURN_SUCCESS ? SUCCESS : FAILED;
    }

    Result PackageImpl::Install(const std::string &packageId, const std::string &version, const NameValues &additionalMetadata, const std::string &fileLocator, ConfigMetaData &configMetadata)
    {
        std::string type, category, appName;
        // Extract additional metadata
        getKeyValue(additionalMetadata, "type", type);
        getKeyValue(additionalMetadata, "category", category);
        getKeyValue(additionalMetadata, "appName", appName);

        INFO("PackageImpl Install, Status : type ", type, " category ", category, " appName ", appName);

        uint32_t result = executor.Install(type, packageId, version, fileLocator, appName, category);
        // The executor will handle the installation process, so we return SUCCESS here
        return result == RETURN_SUCCESS ? SUCCESS : FAILED;
    }

    Result PackageImpl::Lock(const std::string &packageId, const std::string &version, std::string &unpackedPath, ConfigMetaData &appConfig, NameValues &additionalLocks)
    {
        uint32_t result = executor.GetAppInstalledPath(packageId, version, unpackedPath); // Assuming appPath is the unpacked path

        if (result == RETURN_SUCCESS)
        {
            INFO("Locking for app ", packageId, ", Path is ", unpackedPath);
            std::string configPath, path;
            if (executor.GetAppConfigPath(path, configPath) == RETURN_SUCCESS)
            {
                DEBUG("Config path for app ", packageId, " is ", configPath);

                if (populateConfigValues(configPath, appConfig))
                {
                    appConfig.appType = INTERACTIVE;
                    appConfig.wanLanAccess = true;
                    appConfig.thunder = true;
                }
            }
        }
        return result == RETURN_SUCCESS ? SUCCESS : FAILED; // Locking is not implemented in this context, so we return SUCCESS
    }

    Result PackageImpl::Uninstall(const std::string &packageId)
    {
        std::string uninstallType = "full"; // Assuming full uninstall
        DataStorage::AppDetails appDetails;
        INFO("Retrieving app details for packageId: ", packageId);
        executor.GetAppDetails(packageId, appDetails);

        uint32_t result = executor.Uninstall(appDetails.type, packageId, appDetails.version, uninstallType);
        return result == RETURN_SUCCESS ? SUCCESS : FAILED;
    }

    std::shared_ptr<packagemanager::IPackageImpl> IPackageImpl::instance()
    {

        std::shared_ptr<packagemanager::IPackageImpl> packageImpl =
            std::make_shared<packagemanager::PackageImpl>();

        return packageImpl;
    }
    bool PackageImpl::populateConfigValues(const std::string &configjsonfile, ConfigMetaData &configMetadata /* out*/)
    {
        DEBUG("Populating config values from: ", configjsonfile);
        try
        {
            boost::property_tree::ptree pt;
            boost::property_tree::read_json(configjsonfile, pt);

            boost::property_tree::ptree envObject = pt.get_child("process.env", boost::property_tree::ptree());
            std::vector<std::string> envVars;
            for (const auto &item : envObject)
            {
                DEBUG("Adding environment variable: ", item.second.data());
                envVars.push_back(item.second.data());
            }
            configMetadata.envVars = envVars;
            envObject = pt.get_child("process.args", boost::property_tree::ptree());
            std::string launchCommand;
            for (const auto &item : envObject)
            {
                launchCommand += item.second.data() + " ";
            }
            DEBUG(" Adding launch command: ", launchCommand);
            configMetadata.command = launchCommand;

            return true;
        }
        catch (const std::exception &e)
        {
            ERROR("Error populating config values: ", e.what());
            return false;
        }
    }
}