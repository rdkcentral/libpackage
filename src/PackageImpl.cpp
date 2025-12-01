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


#include <ralf/Logging.h>
#include <ralf/Package.h>

using namespace LIBRALF_NS;

namespace packagemanager
{
    Result PackageImpl::Initialize(const std::string &configString, ConfigMetadataArray &appMetaMap)
    { 
	// Initialize the executor
        uint32_t result = ralfexecutor.Configure(configString); // Assuming empty config for now, can be replaced with actual config string
        if (result != RETURN_SUCCESS)
        {
            ERROR("Failed to configure executor, Status : ", result);
            return FAILED;
        }
        std::vector<RalfExecutor::AppDetails> appsDetailsList;
        result = ralfexecutor.GetAppDetailsList( appsDetailsList);
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
                INFO("Config metadata populated for app: ", details.id);
            }
            else
            {
                ERROR("Failed to populate config metadata for app: ", details.id);
            }
        }
        return result == RETURN_SUCCESS ? SUCCESS : FAILED;
    }

    Result PackageImpl::Install(const std::string &packageId, const std::string &version, const NameValues &additionalMetadata, const std::string &fileLocator, ConfigMetaData &configMetadata)
    {
       std::string type, category, appName;
	   uint32_t result = FAILED;
#if 1 
       result = ralfexecutor.Install( packageId, version, fileLocator);
 #endif
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
        INFO("Populating config values for : ", packageId);
        std::string metaDataPath;
        uint32_t result = ralfexecutor.GetAppPath(packageId, version,metaDataPath); //App Meta data also stored
        configMetadata.appPath = metaDataPath; //This is path for metadata.json , do we need to full Name ??
	return true;
    }

    Result PackageImpl::GetFileMetadata(const std::string &fileLocator, std::string &packageId, std::string &version, ConfigMetaData &configMetadata)
    {
        INFO("PackageImpl GetFileMetadata, packageId: ", packageId, " version: ", version, " fileLocator: ", fileLocator);
        return populateConfigValues(packageId, version, configMetadata) ? SUCCESS : FAILED;
    }
}
