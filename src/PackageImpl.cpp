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
        LOG("PackageImpl  initialized, Status : ", result);
        std::vector<DataStorage::AppDetails> appsDetailsList;
        const std::string type = "";
        const std::string id = "";
        const std::string version = "";
        const std::string appName = "";
        const std::string category = "";

        result = executor.GetAppDetailsList(type, id, version, appName, category, appsDetailsList);
        if (result != Executor::ReturnCodes::ERROR_NONE)
        {
            LOG("Failed to retrieve app details list, Status : ", result);
            return FAILED;
        }
        LOG("Retrieved ", appsDetailsList.size(), " apps.");
        for (auto details : appsDetailsList)
        {
            ConfigMetaData configMetaData;
            std::string configPath;
            executor.GetAppConfigPath(details.id, details.version, configPath);
            LOG(details.appName, " is  installed at: ", configPath);

            if (populateConfigValues(configPath, configMetaData))
            {
                configMetaData.appType = INTERACTIVE;
                configMetaData.wanLanAccess = true;
                configMetaData.thunder = true;
                configMetaData.appPath = "/"; // Assuming this is referring to CWD
                ConfigMetadataKey app = {details.id, details.version};
                configMetadata[app] = configMetaData;
                LOG("Config metadata populated for app: ", details.appName);
            }
            else
            {
                LOG("Failed to populate config metadata for app: ", details.appName);
            }
        }
        return result == Executor::ReturnCodes::ERROR_NONE ? SUCCESS : FAILED;
    }

    Result PackageImpl::Install(const std::string &packageId, const std::string &version, const NameValues &additionalMetadata, const std::string &fileLocator, ConfigMetaData &configMetadata)
    {
        std::string type, category, appName;
        // Extract additional metadata
        getKeyValue(additionalMetadata, "type", type);
        getKeyValue(additionalMetadata, "category", category);
        getKeyValue(additionalMetadata, "appName", appName);

        LOG("PackageImpl Install, Status : type ", type, " category ", category, " appName ", appName);

        uint32_t result = executor.Install(type, packageId, version, fileLocator, appName, category);
        // The executor will handle the installation process, so we return SUCCESS here
        return result == Executor::ReturnCodes::ERROR_NONE ? SUCCESS : FAILED;
    }

    Result PackageImpl::Uninstall(const std::string &packageId)
    {
        std::string uninstallType = "full"; // Assuming full uninstall
        std::string handle;
        DataStorage::AppDetails appDetails;
        LOG("Retrieving app details for packageId: ", packageId);
        executor.GetAppDetails(packageId, appDetails);
        // LOG("Uninstalling app: {", appDetails.type,", ", appDetails.version, ", ", appDetails.appName, "}");
        uint32_t result = executor.Uninstall(appDetails.type, packageId, appDetails.version, uninstallType, handle);
        LOG("Uninstall handle: ", handle);
        return result == Executor::ReturnCodes::ERROR_NONE ? SUCCESS : FAILED;
    }

    std::shared_ptr<packagemanager::IPackageImpl> IPackageImpl::instance()
    {

        std::shared_ptr<packagemanager::IPackageImpl> packageImpl =
            std::make_shared<packagemanager::PackageImpl>();

        return packageImpl;
    }
    bool PackageImpl::populateConfigValues(const std::string &configjsonfile, ConfigMetaData &configMetadata /* out*/)
    {
        LOG("Populating config values from: ", configjsonfile);
        try
        {
            boost::property_tree::ptree pt;
            boost::property_tree::read_json(configjsonfile, pt);

            boost::property_tree::ptree envObject = pt.get_child("process.env", boost::property_tree::ptree());
            std::vector<std::string> envVars;
            for (const auto &item : envObject)
            {
                LOG("Adding environment variable: ", item.second.data());
                envVars.push_back(item.second.data());
            }
            configMetadata.envVars = envVars;
            envObject = pt.get_child("process.args", boost::property_tree::ptree());
            std::string launchCommand;
            for (const auto &item : envObject)
            {
                launchCommand += item.second.data() + " ";
            }
            LOG(" Adding launch command: ", launchCommand);
            configMetadata.command = launchCommand;

            return true;
        }
        catch (const std::exception &e)
        {
            LOG("Error populating config values: ", e.what());
            return false;
        }
    }
}