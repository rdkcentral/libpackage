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

#include "Executor.h"

#include "Archives.h"
#include "Config.h"
#include "Debug.h"
#include "Filesystem.h"
#include "SqlDataStorage.h"

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
    namespace // anonymous
    {

        std::string extractFilename(const std::string &uri)
        {
            std::size_t found = uri.find_last_of('/');
            return uri.substr(found + 1);
        }

        // TODO some enhancements
        // - replace type,id,version triplet with AppId - make it less error prone (named arguments?)
        // - create AppConfig that will take Config and encapsulate path creation - at present it's
        //   easy easy to make mistake, similar code is different places
        struct AppId
        {
            std::string id;
            std::string version;
        };

        std::ostream &operator<<(std::ostream &out, const AppId &app)
        {
            return out << "app[" << app.id << ":" << app.version << "]";
        }

        std::vector<AppId> scanDirectories(const std::string &appsPath, bool scanDataStorage)
        {
            std::vector<AppId> apps;
            std::string currentPath;

            auto appsPaths = Filesystem::getSubdirectories(appsPath);
            for (auto &idPath : appsPaths)
            {

                currentPath = appsPath + idPath + '/';
                if (Filesystem::isEmpty(currentPath))
                {
                    DEBUG("empty dir: ", currentPath, " removing");
                    Filesystem::removeDirectory(currentPath);
                    continue;
                }

                AppId appId{};
                appId.id = idPath;

                if (scanDataStorage)
                {
                    apps.emplace_back(appId);
                }
                else
                {
                    auto verSubPaths = Filesystem::getSubdirectories(currentPath);
                    for (auto &verSubPath : verSubPaths)
                    {

                        currentPath = appsPath + idPath + '/' + verSubPath + '/';
                        if (Filesystem::isEmpty(currentPath))
                        {
                            DEBUG("empty dir: ", currentPath, " removing");
                            Filesystem::removeDirectory(currentPath);
                            continue;
                        }

                        AppId appVer = appId;
                        appVer.version = verSubPath;

                        apps.emplace_back(appVer);
                    }
                }
            }
            return apps;
        }

    } // namespace anonymous

    uint32_t Executor::Configure(const std::string &configString)
    {
        INFO("[Executor::Configure] config: '", configString, "'");
        config = Config{configString};

        auto result{RETURN_SUCCESS};
        if (config.getAppsPath().empty() || config.getDatabasePath().empty())
        {
            ERROR("[Executor::Configure] Apps path or database path is empty");
            return RETURN_ERROR;
        }
        try
        {
            handleDirectories();
            initializeDataBase(config.getDatabasePath());
            doMaintenance();
            INFO("[Executor::Configure] configuration done");
        }
        catch (std::exception &error)
        {
            ERROR("[Executor::Configure] Unable to configure executor: ", error.what());
            return RETURN_ERROR;
        }
        return result;
    }

    uint32_t Executor::Install(const std::string &type,
                               const std::string &id,
                               const std::string &version,
                               const std::string &url,
                               const std::string &appName,
                               const std::string &category)
    {
        INFO("[ Executor::Install] type=", type, " id=", id, " version=", version, " url=", url, " appName=", appName, " cat=", category);

        if (type.empty() || id.empty() || version.empty())
        {
            ERROR("[Executor::Install] Invalid parameters!");
            return RETURN_ERROR;
        }

        if (!Filesystem::isAcceptableFilePath(id) || !Filesystem::isAcceptableFilePath(version))
        {
            ERROR("[Executor::Install] Invalid file or path name!");
            return RETURN_ERROR;
        }

        LockGuard lock(taskMutex);

        if (isAppInstalled(type, id, version))
        {
            ERROR("[Executor::Install] App is already installed!");
            return RETURN_ERROR;
        }

        try
        {
            if (dataBase->GetTypeOfApp(id) != type)
            {
                ERROR("[Executor::Install] In the DB id '", id, "' is already used with another type! App id must be unique.");
                return RETURN_ERROR;
            }
        }
        catch (const SqlDataStorageError &)
        {
            // fine, no problem, not a single version of app(id) installed yet
        }
        bool status = extract(type, id, version, url, appName, category);
        return status ? RETURN_SUCCESS : RETURN_ERROR;
    }

    uint32_t Executor::Uninstall(const std::string &type,
                                 const std::string &id,
                                 const std::string &version,
                                 const std::string &uninstallType)
    {
        INFO("[Executor::Uninstall] type=", type, " id=", id, " version=", version, " uninstallType=", uninstallType);

        // TODO what are param requirements?
        if (uninstallType != "full" && uninstallType != "upgrade")
        {
            ERROR("[Executor::Uninstall] uninstallType must be 'full' or 'upgrade'");
            return RETURN_ERROR;
        }

        // If an app was uninstalled earlier with uninstallType=upgrade, then the
        // app record will still be inside the database. Also the data storage dir will
        // still exist. Allow the uninstallation of these artifacts with "if test" below.
        // Second "if test" is the uninstallation of the usual case:First packageg an app
        // of specific version.
        if (version.empty() && !type.empty() && !id.empty() && uninstallType == "full")
        {
            // verify that such an app record exists
            if (dataBase->GetDataPaths(type, id).size() == 0)
            {
                ERROR("[Executor::Uninstall] No app data found for type=", type, " id=", id);
                return RETURN_ERROR;
            }
            // only allowed when no specific version of app installed anymore
            // if there are: the usual uninstall with a specific version should be called
            if (dataBase->GetAppsPaths(type, id, "").size() > 0)
            {
                ERROR("[Executor::Uninstall] There are still versions of app installed for type=", type, " id=", id);
                return RETURN_ERROR;
            }
        }
        else if (!isAppInstalled(type, id, version))
        {
            ERROR("[Executor::Uninstall] App not installed: type=", type, " id=", id, " version=", version);
            return RETURN_ERROR;
        }
        INFO("[Executor::Uninstall] We are good to uninstall");
        LockGuard lock(taskMutex);

        if (std::find(lockedApps.begin(), lockedApps.end(), std::make_pair(id, version)) != lockedApps.end())
        {
            ERROR("Cannot uninstall app because of lock!");
            return RETURN_ERROR;
        }
        INFO("[Executor::Uninstall] App is not locked, proceeding with uninstall");

        doUninstall(type, id, version, uninstallType);
        return RETURN_SUCCESS;
    }

    uint32_t Executor::GetStorageDetails(const std::string &type,
                                         const std::string &id,
                                         const std::string &version,
                                         Filesystem::StorageDetails &details)
    {
        namespace fs = Filesystem;

        try
        {
            // if all params are empty then the overall disk usage is calculated
            if (type.empty() && id.empty() && version.empty())
            {
                INFO("[Executor::GetStorageDetails] Calculating overall usage");
                details.appPath = config.getAppsPath();
                details.appUsedKB = std::to_string((fs::getDirectorySpace(config.getAppsPath()) + fs::getDirectorySpace(config.getAppsTmpPath())) / 1024);
            }
            else if (!id.empty())
            {
                // When specific id is passed, calculate disk usage for this app.
                // Type is optional since id is unique and sufficient. But if passed, it must match.
                // If version is also passed: calculate the app size for this version, else not reported.
                // Data storage size is always reported because this is version independent
                INFO("[Executor::GetStorageDetails] Calculating usage for: type = ", type, " id = ", id, " version = ", version);
                if (!version.empty())
                {
                    std::vector<std::string> appsPaths = dataBase->GetAppsPaths(type, id, version);
                    if (appsPaths.empty())
                    {
                        // return error when app not found
                        return RETURN_ERROR;
                    }
                    unsigned long long appUsedKB{};
                    // In Stage 1 there will be only one entry here
                    for (const auto &i : appsPaths)
                    {
                        details.appPath = config.getAppsPath() + i;
                        appUsedKB += fs::getDirectorySpace(details.appPath);
                    }
                    details.appUsedKB = std::to_string(appUsedKB / 1024);
                }
            }
            else
            {
                return RETURN_ERROR;
            }
        }
        catch (std::exception &error)
        {
            ERROR("[Executor::GetStorageDetails] Unable to retrieve storage details: ", error.what());
            return RETURN_ERROR;
        }
        return RETURN_SUCCESS;
    }

    uint32_t Executor::GetAppDetails(
        const std::string &id,
        DataStorage::AppDetails &appDetails) const
    {
        if (id.empty())
        {
            ERROR("[Executor::GetAppDetails] Need app id to get details");
            return RETURN_ERROR;
        }

        try
        {

            DataStorage::AppDetails details = dataBase->GetAppDetails(id);
            appDetails.type = details.type;
            appDetails.id = details.id;
            appDetails.version = details.version;
            appDetails.appName = details.appName;
            appDetails.category = details.category;
            appDetails.url = details.url;
        }
        catch (std::exception &error)
        {
            ERROR("[Executor::GetAppDetails] Unable to get Application details: ", error.what());
            return RETURN_ERROR;
        }
        return RETURN_SUCCESS;
    }
    uint32_t Executor::GetAppDetailsList(const std::string &type,
                                         const std::string &id,
                                         const std::string &version,
                                         const std::string &appName,
                                         const std::string &category,
                                         std::vector<DataStorage::AppDetails> &appsDetailsList) const
    {
        INFO("[Executor::GetAppDetailsList] for id : ", id, ", version : ", version);
        try
        {
            appsDetailsList = dataBase->GetAppDetailsListOuterJoin(type, id, version, appName, category);
            INFO("[Executor::GetAppDetailsList] Retrieved ", appsDetailsList.size(), " app details");
        }
        catch (std::exception &error)
        {
            ERROR("Unable to get Applications details: ", error.what());
            return RETURN_ERROR;
        }
        return RETURN_SUCCESS;
    }

    uint32_t Executor::SetMetadata(const std::string &type,
                                   const std::string &id,
                                   const std::string &version,
                                   const std::string &key,
                                   const std::string &value)
    {
        if (type.empty() || id.empty() || version.empty() || key.empty())
        {
            ERROR("[Executor::SetMetadata] Invalid parameters");
            return RETURN_ERROR;
        }

        try
        {
            dataBase->SetMetadata(type, id, version, key, value);
        }
        catch (std::exception &error)
        {
            ERROR("[Executor::SetMetadata] Unable to set metadata: ", error.what());
            return RETURN_ERROR;
        }
        return RETURN_SUCCESS;
    }

    uint32_t Executor::ClearMetadata(const std::string &type,
                                     const std::string &id,
                                     const std::string &version,
                                     const std::string &key)
    {
        if (type.empty() || id.empty() || version.empty())
        {
            ERROR("[Executor::ClearMetadata] Invalid parameters");
            return RETURN_ERROR;
        }

        try
        {
            dataBase->ClearMetadata(type, id, version, key);
        }
        catch (std::exception &error)
        {
            ERROR("[Executor::ClearMetadata] Unable to clear metadata: ", error.what());
            return RETURN_ERROR;
        }
        return RETURN_SUCCESS;
    }

    uint32_t Executor::GetMetadata(const std::string &type,
                                   const std::string &id,
                                   const std::string &version,
                                   DataStorage::AppMetadata &metadata) const
    {
        if (type == DAC_CONFIG_MIMETYPE && id == DAC_CONFIG_APP_ID && version == DAC_CONFIG_APP_VERSION)
        {
            metadata.appDetails.id = DAC_CONFIG_APP_ID;
            metadata.appDetails.type = DAC_CONFIG_MIMETYPE;
            metadata.appDetails.version = DAC_CONFIG_APP_VERSION;
            metadata.metadata.emplace_back(DACBUNDLEPLATFORMNAMEOVERRIDE_KEY_NAME,
                                           config.getDacBundlePlatformNameOverride());
            metadata.metadata.emplace_back(DACBUNDLEFIRMWARECOMPATIBILITYKEY_KEY_NAME,
                                           config.getDacBundleFirmwareCompatibilityKey());
            metadata.metadata.emplace_back(CONFIG_URL_KEY_NAME,
                                           config.getConfigUrl());
            return RETURN_SUCCESS;
        }

        if (type.empty() || id.empty() || version.empty())
        {
            ERROR("[Executor::GetMetadata] Invalid parameters");
            return RETURN_ERROR;
        }

        try
        {
            metadata = dataBase->GetMetadata(type, id, version);
        }
        catch (std::exception &error)
        {
            ERROR("[Executor::GetMetadata] Unable to get metadata: ", error.what());
            return RETURN_ERROR;
        }
        return RETURN_SUCCESS;
    }

    void Executor::handleDirectories()
    {
#if LISA_APPS_GID
        Filesystem::createDirectory(config.getAppsPath() + Filesystem::LISA_EPOCH, LISA_APPS_GID, false);
#else
        Filesystem::createDirectory(config.getAppsPath() + Filesystem::LISA_EPOCH);
#endif
        Filesystem::removeAllDirectoriesExcept(config.getAppsPath(), Filesystem::LISA_EPOCH);
    }

    void Executor::initializeDataBase(const std::string &dbPath)
    {
        std::string path = dbPath + Filesystem::LISA_EPOCH + '/';
        Filesystem::ScopedDir dbDir(path);
        dataBase = std::make_unique<packagemanager::SqlDataStorage>(path);
        dataBase->Initialize();
        dbDir.commit();
        INFO("[Executor::initializeDataBase] Database created");
    }

    bool Executor::isAppInstalled(const std::string &type,
                                  const std::string &id,
                                  const std::string &version)
    {
        auto appInstalled{false};
        try
        {
            appInstalled = dataBase->IsAppInstalled(type, id, version);
        }
        catch (std::exception &exc)
        {
            ERROR("[Executor::isAppInstalled] error while checking if app installed: ", exc.what());
        }
        return appInstalled;
    }

    void Executor::importAnnotations(const std::string &type,
                                     const std::string &id,
                                     const std::string &version,
                                     const std::string &appPath)
    {
        if (config.getAnnotationsFile().empty())
        {
            return;
        }

        boost::filesystem::path filepath = appPath;
        filepath /= config.getAnnotationsFile();
        std::ifstream file(filepath.string());
        if (!file.good())
        {
            return;
        }

        if (!file.is_open())
        {
            ERROR("[Executor::importAnnotations] Failed to open ", filepath.string());
            return;
        }

        INFO("[Executor::importAnnotations] Auto importing annotations from ", filepath.string());
        try
        {
            boost::property_tree::ptree pt;
            boost::property_tree::read_json(file, pt);

            if (pt.count("annotations") > 0)
            {
                std::regex pattern(config.getAnnotationsRegex());
                for (const auto &kvp : pt.get_child("annotations"))
                {
                    auto &key = kvp.first;
                    auto &value = kvp.second.data();

                    if (std::regex_search(key, pattern))
                    {
                        DEBUG("[Executor::importAnnotations] Importing ", key, " = ", value, " as metadata");
                        try
                        {
                            dataBase->SetMetadata(type, id, version, key, value);
                        }
                        catch (std::exception &error)
                        {
                            ERROR("[Executor::importAnnotations] Unable to save metadata: ", error.what());
                        }
                    }
                }
            }
        }
        catch (std::exception &error)
        {
            ERROR("[Executor::importAnnotations] Error reading or parsing annotations: ", error.what());
        }
    }

    bool Executor::extract(std::string type,
                           std::string id,
                           std::string version,
                           std::string url,
                           std::string appName,
                           std::string category)
    {
        DEBUG("[Executor::extract] url=", url, " appName=", appName, " cat=", category);

        auto appSubPath = Filesystem::createAppPath(id, version);
        DEBUG("[Executor::extract] appSubPath: ", appSubPath);

        auto tmpPath = config.getAppsTmpPath();
        auto tmpDirPath = tmpPath + appSubPath;
        Filesystem::ScopedDir scopedTmpDir{tmpDirPath};

        // The full file path to the dowloaded app archive is passes as url.
        auto tmpFilePath = url;

        const std::string appsPath = config.getAppsPath() + appSubPath;
        DEBUG("[Executor::extract] creating ", appsPath);
        Filesystem::ScopedDir scopedAppDir{appsPath};

        DEBUG("[Executor::extract] Extracting ", tmpFilePath, "to ", appsPath);
        bool response = Archive::unpackArchive(tmpFilePath, appsPath);

        auto appStorageSubPath = Filesystem::createAppPath(id);

        // We are passing empty URL as we are no longer downloading the app
        //  from the URL, but rather unpacking it from the tmp directory.
        dataBase->AddInstalledApp(type, id, version, "", appName, category, appSubPath, appStorageSubPath);

        // everything went fine, mark app directories to not be removed
        scopedAppDir.commit();

        // auto-import annotations as metadata
        importAnnotations(type, id, version, appsPath);

        doMaintenance();

        DEBUG("[Executor::extract] finished");
        return response;
    }

    void Executor::doUninstall(std::string type, std::string id, std::string version, std::string uninstallType)
    {
        DEBUG("[Executor::doUninstall] type=", type, " id=", id, " version=", version, " uninstallType=", uninstallType);

        if (!version.empty())
        {
            dataBase->RemoveInstalledApp(type, id, version);

            auto appSubPath = Filesystem::createAppPath(id, version);
            auto appPath = config.getAppsPath() + appSubPath;

            DEBUG("[Executor::doUninstall] removing ", appPath);
            Filesystem::removeDirectory(appPath);
        }

        doMaintenance();

        DEBUG("[Executor::doUninstall] finished");
    }
    uint32_t Executor::GetAppInstalledPath(const std::string &id,
                                           const std::string &version, std::string &appPath) const
    {
        DEBUG("GetAppInstalledPath id=", id, " version=", version);
        if (id.empty() || version.empty())
        {
            ERROR("GetAppInstalledPath: id or version is empty");
            return RETURN_ERROR;
        }
        std::vector<std::string> path = dataBase->GetAppsPaths("", id, version);

        if (!path.empty())
        {
            appPath = config.getAppsPath() + path[0];
            DEBUG("GetAppInstalledPath: appPath=", appPath);
        }
        else
        {
            DEBUG("GetAppInstalledPath: No app path found for id=", id, " version=", version);
            return RETURN_ERROR;
        }
        return RETURN_SUCCESS;
    }
    uint32_t Executor::GetAppConfigPath(const std::string &path,
                                        std::string &appPath) const
    {
        DEBUG("GetAppConfigPath installed path=", path);

        appPath = path + "/" + config.getAnnotationsFile();

        return boost::filesystem::exists(appPath) ? RETURN_SUCCESS : RETURN_ERROR;
    }
    void Executor::doMaintenance()
    {
        try
        {
            // clear tmp
            Filesystem::removeDirectory(config.getAppsTmpPath());
            Filesystem::createDirectory(config.getAppsTmpPath());

            // remove installed apps data not present in installed_apps
            auto appsPathRoot = config.getAppsPath() + Filesystem::LISA_EPOCH + '/';
            auto foundApps = scanDirectories(appsPathRoot, false);
            for (const auto &app : foundApps)
            {
                DEBUG(app);
                if (!dataBase->IsAppInstalled("", app.id, app.version))
                {
                    ERROR(app, " not found in installed apps, removing dir");
                    auto path = config.getAppsPath() + Filesystem::createAppPath(app.id, app.version);
                    Filesystem::removeDirectory(path);
                }
            }

            auto appsDetailsList = dataBase->GetAppDetailsListOuterJoin();
            for (const auto &details : appsDetailsList)
            {
                DEBUG("details: ", details.id, ":", details.version);

                auto appPaths = dataBase->GetAppsPaths(details.type, details.id, details.version);

                DEBUG("PATHS APPS:");
                for (const auto &path : appPaths)
                {
                    DEBUG("path: ", path);
                    auto appPath = config.getAppsPath() + path;
                    DEBUG("abs path: ", appPath);

                    bool noAppFiles = Filesystem::directoryExists(appPath) ? Filesystem::isEmpty(appPath) : true;
                    if (noAppFiles)
                    {
                        dataBase->RemoveInstalledApp(details.type, details.id, details.version);
                    }
                }

                auto dataPaths = dataBase->GetDataPaths(details.type, details.id);
            }

#if LISA_APPS_GID
            Filesystem::setPermissionsRecursively(config.getAppsPath(), LISA_APPS_GID, false);
#endif
        }
        catch (std::exception &exc)
        {
            ERROR("ERROR: ", exc.what());
        }
    }
} // namespace  packagemanager
