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
        if (result != RETURN_SUCCESS)
        {
            ERROR("Failed to configure executor, Status : ", result);
            return FAILED;
        }
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

            if (populateConfigValues(details.id, details.version, configMetaData))
            {
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
        INFO("PackageImpl Lock, packageId: ", packageId, " version: ", version);
        if (populateConfigValues(packageId, version, appConfig))
        {
            unpackedPath = appConfig.appPath;
            return SUCCESS;
        }
        ERROR("Failed to lock packageId: ", packageId, " version: ", version);
        return FAILED;
    }

    Result PackageImpl::Uninstall(const std::string &packageId)
    {
        uint32_t result = RETURN_ERROR;
        std::string uninstallType = "full"; // Assuming full uninstall

        if (!packageId.empty())
        {
            DataStorage::AppDetails appDetails;
            INFO("Retrieving app details for packageId: ", packageId);
            result = executor.GetAppDetails(packageId, appDetails);
            if (result == RETURN_SUCCESS)
                result = executor.Uninstall(appDetails.type, packageId, appDetails.version, uninstallType);
            else
                ERROR("Failed to retrieve package details for packageId: ", packageId);
        }
        else
        {
            ERROR("Empty packageId provided for Uninstall");
        }
        return result == RETURN_SUCCESS ? SUCCESS : FAILED;
    }

    std::shared_ptr<packagemanager::IPackageImpl> IPackageImpl::instance()
    {

        std::shared_ptr<packagemanager::IPackageImpl> packageImpl =
            std::make_shared<packagemanager::PackageImpl>();

        return packageImpl;
    }
    bool PackageImpl::populateConfigValues(const std::string &packageId, const std::string &version, ConfigMetaData &configMetadata /* out*/)
    {
        DEBUG("Populating config values for : ", packageId);
        std::string unpackedPath;
        uint32_t result = executor.GetAppInstalledPath(packageId, version, unpackedPath); // Assuming appPath is the unpacked path

        if (result == RETURN_ERROR)
        {
            ERROR("Failed to get installed path for app ", packageId);
            return false;
        }
        configMetadata.appPath = unpackedPath;
        std::string configPath;
        if (executor.GetAppConfigPath(unpackedPath, configPath) == RETURN_ERROR)
        {
            ERROR("Failed to find config path for app ", packageId);
            return false;
        }

        try
        {
            boost::property_tree::ptree pt;
            boost::property_tree::read_json(configPath, pt);

            boost::property_tree::ptree envObject;
            auto envOpt = pt.get_child_optional("process.env");
            std::vector<std::string> envVars;
            if (envOpt)
            {
                envObject = *envOpt;
                for (const auto &item : envObject)
                {
                    DEBUG("Adding environment variable: ", item.second.data());
                    envVars.push_back(item.second.data());
                }
            }
            configMetadata.envVars = envVars;
            auto argsOpt = pt.get_child_optional("process.args");
            std::string launchCommand;
            if (argsOpt)
            {
                envObject = *argsOpt;
                for (const auto &item : envObject)
                {
                    launchCommand += item.second.data() + " ";
                }
            }
            DEBUG(" Adding launch command: ", launchCommand);
            configMetadata.command = launchCommand;
            configMetadata.appType = INTERACTIVE;
            configMetadata.wanLanAccess = true;
            configMetadata.thunder = true;
            return true;
        }
        catch (const std::exception &e)
        {
            ERROR("Error populating config values: ", e.what());
            return false;
        }
    }
    Result PackageImpl::GetFileMetadata(const std::string &fileLocator, std::string &packageId, std::string &version, ConfigMetaData &configMetadata)
    {
        INFO("PackageImpl GetFileMetadata, packageId: ", packageId, " version: ", version, " fileLocator: ", fileLocator);
        return populateConfigValues(packageId, version, configMetadata) ? SUCCESS : FAILED;
    }
}