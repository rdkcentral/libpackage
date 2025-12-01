/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 Liberty Global Service B.V.
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

#include "RalfExecutor.h"

#include "Config.h"
#include "Debug.h"

#include <array>
#include <cassert>
#include <random>
#include <limits>
#include <fstream>
#include <regex>
#include <algorithm>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem.hpp>

#include <ralf/Package.h>
#include "RFileSystem.h"
#include <filesystem>

//#include <nlohmann/json.hpp>

//using namespace LIBRALF_NS;

#ifdef UNIT_TESTS
namespace Core
{
    enum ErrorCodes
    {
        ERROR_NONE = 0,
        ERROR_GENERAL = 1
    };
}
#endif

namespace packagemanager
{
//using namespace LIBRALF_NS;
    uint32_t RalfExecutor::GetAppPath(const std::string &id,
                                           const std::string &version, std::string &appPath) const
    {
    	 DEBUG("GetAppInstalledPath id=", id, " version=", version);
        if (id.empty() || version.empty())
        {
            ERROR("GetAppInstalledPath: id or version is empty");
            return RETURN_ERROR;
        }
        //Package will be copied to this path
        appPath = config.getAppsPath() + "/" + id + "/" + version;
        return RETURN_SUCCESS;
    }
    void  RalfExecutor::metaDataToConfigSpec(const PackageMetaData &metaData)
    {
     WARNING("Package Meta Data\n");   
     WARNING("id= ",metaData.id(),"\nversion=" ,metaData.version().toString(),   "\nversionName= " , metaData.versionName());   
     fprintf(stderr,"\n");   
#if 0
    nlohmann::ordered_json spec(nlohmann::json::value_t::object);
 
     spec["id"] = metaData.id();
    spec["version"] = metaData.version().toString();
    if (!metaData.versionName().empty())
        spec["versionName"] = metaData.versionName();
    if (metaData.title().has_value())
        spec["name"] = metaData.title().value();

    switch (metaData.type())
    {
        case PackageType::Application:
            spec["packageType"] = "application";
            spec["runtimeType"] = metaData.applicationInfo()->runtimeType();
            break;
        case PackageType::Service:
            spec["packageType"] = "service";
            spec["runtimeType"] = metaData.serviceInfo()->runtimeType();
            break;
        case PackageType::Runtime:
            spec["packageType"] = "runtime";
            break;
        case PackageType::Base:
            spec["packageType"] = "base";
            break;
        default:
            spec["packageType"] = "unknown";
            break;
    }

    spec["entryPointPath"] = metaData.entryPointPath();


    spec["dependencies"] = createDependenciesJson(metaData);

    if (metaData.type() == PackageType::Application)
        spec["permissions"] = createPermissionsJson(metaData.applicationInfo()->permissions());
    else if (metaData.type() == PackageType::Service)
        spec["permissions"] = createPermissionsJson(metaData.serviceInfo()->permissions());
#endif
     }

   bool  RalfExecutor::dumpPackageMetadata (std::string fileLocator) 
    {
        // Result<Package> package;

         Result<Package> package  = Package::openWithoutVerification( fileLocator.c_str(), Package::OpenFlags::None);
        if (package.is_error())
        {  
         fprintf(stderr, "Error: failed to open package '%s' - %s\n", fileLocator.c_str(), package.error().what());
         return false; 
       }
        const auto packageFormat = package.value().format(); //Need to check format is corrct or not TODO
        const auto metadata = package.value().metaData();
        if (!metadata)
        {
            fprintf(stderr, "Error: failed to read package meta data - %s\n", metadata.error().what());
            return false;
        }
            fprintf(stderr, "PASS  read package meta data - %s\n", metadata.error().what());
        metaDataToConfigSpec(metadata.value());
       //std::cout << metaDataToConfigSpec(metadata.value()) << std::endl;
 
	    return true;
    }


    uint32_t RalfExecutor::Configure(const std::string &configString)
    {
        INFO("[RalfExecutor::Configure] config: '", configString, "'");
        config = Config{configString};

        auto result{RETURN_SUCCESS};
        if (config.getAppsPath().empty() )//|| config.getDatabasePath().empty())
        {
            ERROR("Ralf[Executor::Configure] Apps path is empty");
            return RETURN_ERROR;
        }
           //Create 
            createDirectories();
        return result;
    }

    uint32_t RalfExecutor::Install(
                               const std::string &id,
                               const std::string &version,
                               const std::string &url)
    {
        INFO("[ Executor::Install]  id=", id, " version=", version, " url=", url);
        //TODO Normalize filepath 
        if (url.empty() || id.empty() || version.empty() || !RFileSystem::FileExist(url))
        {
            ERROR("Ralf[Executor::Install] Invalid parameters ! ");
            return RETURN_ERROR;
        }
         //TODO Lock for copy  
         //iTODO If app already installed 
    
    	bool status = false;
        std::string appPath;
        uint32_t result = GetAppPath(id, version,appPath);
        INFO(" AppPath for this package: ",appPath);
        //if appPath exists and package present, app is alresdy installed 
        if( !RFileSystem::CheckDirectoryAndFile(appPath,url)) 
        {
            //if new app, create dir
            RFileSystem::CreateDirectoryr(appPath);  
            if(RFileSystem::CopyFile(url,appPath,id)) 
            {
            //Store package name , so that we can retrive later 
            status= true;
            } 
            else {
            INFO("Package id: ", id, " version: ", version, " unknown error  ");
            return RETURN_ERROR;
            }
        }
        else 
        {
        INFO("Package id: ", id, " version: ", version, " Already Installed ");
       return RETURN_SUCCESS;
        }
        //create meta data for the package 
        status =  dumpPackageMetadata(url);     
        return status ? RETURN_SUCCESS : RETURN_ERROR;
   } 
    uint32_t RalfExecutor::Uninstall(const std::string &type,
                                 const std::string &id,
                                 const std::string &version,
                                 const std::string &uninstallType)
    {
        INFO("[Executor::Uninstall] type=", type, " id=", id, " version=", version, " uninstallType=", uninstallType);
 #if 0
#endif
        return RETURN_SUCCESS;
    }

    //Get All installed App details 
    std::vector<RalfExecutor::AppDetails>  RalfExecutor:: GetAppInfo(const std::string& pathString) const
    {
     std::vector<AppDetails> appsList;
     std::filesystem::path targetPath(pathString);
     if (!std::filesystem::is_directory(targetPath)) 
     {
        std::cerr << "Error: The provided path is not a directory or does not exist." << std::endl;
        return appsList;
     }
     // Iterate through all entries in the directory (non-recursive)
     for (const auto& entry : std::filesystem::directory_iterator(targetPath)) 
     {
        if (std::filesystem::is_directory(entry.status())) 
        {
            AppDetails details;
           //std::cout << "APP ID :  " <<  entry.path().filename().string() << std::endl ;//<< "Depth:"<<subEntry.depth() << std::endl;
            for (const auto& subEntry : std::filesystem::directory_iterator(entry.path())) 
            {
             // Skips "." and ".." automatically
            //std::cout << "SubDir :  " << subEntry.path().filename().string() << std::endl;//<< "Depth:"<<subEntry.depth() << std::endl;
             if (std::filesystem::is_directory(subEntry.status())) 
              {
                details.id = entry.path().filename().string();
                details.version = subEntry.path().filename().string();
                appsList.push_back(details);
              }
             }
         }
      }
         return appsList;
    }


    uint32_t RalfExecutor::GetAppDetailsList(
                                         std::vector<RalfExecutor::AppDetails> &appsDetailsList) const
    {
#if 1
        try
        {
            appsDetailsList = GetAppInfo(config.getAppsPath());
            INFO("[RalfExecutor::GetAppDetailsList] Retrieved ", appsDetailsList.size(), " app details");
        }
        catch (std::exception &error)
        {
            ERROR("Unable to get Applications details: ", error.what());
            return RETURN_ERROR;
        }
#endif
        return RETURN_SUCCESS;
    }



    void RalfExecutor::createDirectories()
    {
       std::error_code ec; 
       if (std::filesystem::create_directories(config.getAppsPath(), ec)) {
        INFO("Directory created succesfully ",config.getAppsPath());
       } else {
        if (ec.value() == EEXIST) { // Directory already exists
           WARNING("Directory ",config.getAppsPath()), "already exists";
        } else {
           ERROR("Create Directory ",config.getAppsPath(), "failed:",ec.message() );
        }
       } 
    }


    void RalfExecutor::doUninstall(std::string id, std::string version)
    {
    }
    uint32_t RalfExecutor::GetAppConfigPath(const std::string &path,
                                        std::string &appPath) const
    {
        DEBUG("GetAppConfigPath installed path=", path);
#if 0
        appPath = path + "/" + config.getAnnotationsFile();

        return boost::filesystem::exists(appPath) ? RETURN_SUCCESS : RETURN_ERROR;
#endif
	return true;
    }
} // namespace  packagemanager
