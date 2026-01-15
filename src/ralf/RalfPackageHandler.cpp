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

#include "RalfPackageImpl.h"
#include <iostream>
#include <filesystem>
#include <ralf/PackageMount.h>
#include <ralf/PackageMetaData.h>
#include <ralf/VersionNumber.h>
#include <json/json.h>
#include <fstream>
#ifdef ENABLE_LOCAL_MOUNT
#include <sys/mount.h>
#endif

#define DISABLE_DEPENDENCY_CHECK
namespace packagemanager
{
    std::string RalfPackageImpl::AppInstallationPath = DAC_APP_PATH;
    std::string RalfPackageImpl::RalfPackage = "package.ralf";

    int RalfPackageImpl::getInstalledPackages(std::vector<std::string> &pacakgeList)
    {
        std::cout << "[libPackage] Looking for installed packages in  " << AppInstallationPath << std::endl;
        for (const auto &entry : std::filesystem::recursive_directory_iterator(AppInstallationPath))
        {
            if (std::filesystem::is_regular_file(entry.path()) && entry.path().filename() == RalfPackage)
            {
                pacakgeList.push_back(entry.path().string());
            }
        }
        return pacakgeList.size();
    }

    bool RalfPackageImpl::identifyDependencyVersion(const std::string &depPackageId, const ralf::VersionConstraint &depPackageVersion, std::string &depInstalledVersion)
    {
        for (const auto &pkgInfo : mInstalledPackages)
        {
            if (pkgInfo->first == depPackageId)
            {
                // See of the package version associated works
                const auto &pkgversion = pkgInfo->second;
                auto result = ralf::VersionNumber::fromString(pkgversion);
                if (!result)
                {
                    std::cerr << "[libPackage] Failed to parse version: " << pkgversion << std::endl;
                    continue;
                }
                ralf::VersionNumber versionNumber = result.value();
                if (depPackageVersion.isSatisfiedBy(versionNumber))
                {
                    depInstalledVersion = pkgversion;
                    return true;
                }
            }
        }
        return false;
    }

    void RalfPackageImpl::getPackageIdAndVersionFromRalfPackage(const std::string &packagePath, std::string &appId, std::string &appVersion)
    {
        std::filesystem::path p(packagePath);
        auto parentPath = p.parent_path();
        appVersion = parentPath.filename().string();
        auto grandParentPath = parentPath.parent_path();
        appId = grandParentPath.filename().string();
    }

    std::shared_ptr<IPackageImpl> IPackageImpl::instance()
    {
        std::shared_ptr<IPackageImpl> packageImpl = std::make_shared<RalfPackageImpl>();
        return packageImpl;
    }

    /**
     * The following activities are performed.
     * 1. Make sure the app installation path exists
     * 2. If path already exists, read the metadata of all installed packages and populate in configMetadata
     */
    Result RalfPackageImpl::Initialize(const std::string &configStr, ConfigMetadataArray &aConfigMetadata)
    {
        std::cout << "[libPackage] RalfPackageImpl::Initialize called with config: " << configStr << std::endl;
        // Check if AppInstallationPath exists
        if (!std::filesystem::exists(AppInstallationPath))
        {
            std::cout << "[libPackage] App installation path does not exist. Creating: " << AppInstallationPath << std::endl;
            std::filesystem::create_directories(AppInstallationPath);
        }
        else
        {
            std::vector<std::string> installedPackages;
            // Let us get package metadata of all installed packages
            auto count = getInstalledPackages(installedPackages);
            std::cout << "[libPackage] Found " << count << " installed packages." << std::endl;

            for (auto packagePath : installedPackages)
            {
                std::string appId, appVersion;
                getPackageIdAndVersionFromRalfPackage(packagePath, appId, appVersion);
                mInstalledPackages.push_back(std::make_unique<ConfigMetadataKey>(std::make_pair(appId, appVersion)));
                std::cout << "[libPackage] Found installed package: " << appId << ", version: " << appVersion << std::endl;
                ConfigMetaData configMetadata;
                configMetadata.appPath = std::filesystem::path(packagePath);

                ConfigMetadataKey appKey = {appId, appVersion};
                aConfigMetadata[appKey] = configMetadata;
            }
        }
        return Result::SUCCESS;
    }

    Result RalfPackageImpl::Install(const std::string &packageId, const std::string &version, const NameValues &additionalMetadata, const std::string &fileLocator, ConfigMetaData &configMetadata)
    {
        std::cout << "[libPackage] RalfPackageImpl::Install called with packageId: " << packageId << ", version: " << version << ", fileLocator: " << fileLocator << std::endl;
        // Step 1: Verify the package
        // This is a TODO

        // Step 2: Get Dependency data for the package. If that is not installed return failure.
        auto package = ralf::Package::openWithoutVerification(fileLocator);

        if (package)
        {
#ifndef DISABLE_DEPENDENCY_CHECK

            std::cout << "[libPackage][DEPENDENCY_CHECK] Successfully opened package: " << fileLocator << std::endl;
            auto pkgMetadata = package->metaData();
            if (pkgMetadata)
            {
                auto dependencies = pkgMetadata->dependencies();
                for (const auto &dependency : dependencies)
                {
                    // Identify the dependency
                    std::string depPackageId = dependency.first;
                    ralf::VersionConstraint depPkgVersion = dependency.second;
                    std::string depInstalledVersion;

                    if (!identifyDependencyVersion(depPackageId, depPkgVersion, depInstalledVersion))
                    {
                        std::cerr << "[libPackage] [DEPENDENCY_CHECK] Failed to identify dependency version for package: " << depPackageId << std::endl;
                        return Result::FAILED;
                    }
                }
            }
            else
            {
                // Log error
                std::cerr
                    << "[libPackage] [DEPENDENCY_CHECK] Failed to read package metadata: " << pkgMetadata.error().what() << std::endl;
                return Result::FAILED;
            }
#endif // DISABLE_DEPENDENCY_CHECK
        }
        else
        {
            // Log error
            std::cerr
                << "[libPackage] Failed to open  package: " << fileLocator << " : " << package.error().what() << std::endl;
            return Result::FAILED;
        }
        std::cout << "[libPackage] Successfully identified dependencies for package: " << fileLocator << std::endl;

        // Step 3: Create the directory structure
        auto packagePath = std::filesystem::path(AppInstallationPath) / packageId / version;
        std::filesystem::create_directories(packagePath);

        // Step 4: Copy the package to the installation directory
        auto destRalfPackagePath = packagePath / RalfPackage;
        try
        {
            std::filesystem::copy_file(fileLocator, destRalfPackagePath, std::filesystem::copy_options::overwrite_existing);
            auto appPath = destRalfPackagePath.string();
            configMetadata.appPath = std::move(appPath);
            std::cout << "[libPackage] Installed package to: " << configMetadata.appPath << std::endl;
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            // Log error
            std::cerr
                << "[libPackage] Error installing package: " << e.what() << std::endl;
            return Result::FAILED;
        }
        std::unique_ptr<ConfigMetadataKey> appIdVer = std::make_unique<ConfigMetadataKey>(std::make_pair(packageId, version));
        mInstalledPackages.push_back(std::move(appIdVer));
        return Result::SUCCESS;
    }
    Result RalfPackageImpl::Uninstall(const std::string &packageId)
    {
        std::cout << "[libPackage] RalfPackageImpl::Uninstall called with packageId: " << packageId << std::endl;
        // For the time being, we have to remove all the files in the package installation path, until we get a version with specific version to uninstall
        auto packagePath = std::filesystem::path(AppInstallationPath) / packageId;
        try
        {
            std::filesystem::remove_all(packagePath);
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            // Log error
            std::cerr
                << "[libPackage] Error uninstalling package: " << e.what() << std::endl;
            return Result::FAILED;
        }
        return Result::SUCCESS;
    }

    /**
     * The following steps are performed
     * 1. Get dependency list first.
     * For each dependency
     *  2. See if the packageInformation file is present. If so this package is already mounted once. No need to go any deeper.
     *  3. If not present, open the package, read the metadata and identify dependencies and dump the dependency data in packageInformation file.
     *  4. Check if the package is already mounted. IF so we need to increase the mount count of each dependency package.
     * 5. If not mounted, mount the package and all its dependencies and set mount count to 1.
     * 6. Return the mount point of the main package.
     */

    Result RalfPackageImpl::Lock(const std::string &packageId, const std::string &version, std::string &unpackedPath, ConfigMetaData &configMetadata, NameValues &additionalLocks)
    {
        std::cout << "[libPackage] RalfPackageImpl::Lock called with packageId: " << packageId << ", version: " << version << std::endl;

        auto packagePath = std::filesystem::path(AppInstallationPath) / packageId / version / RalfPackage;
        auto package = ralf::Package::openWithoutVerification(packagePath);
        if (!package)
        {
            std::cerr << "[libPackage] Failed to open package for locking: " << package.error().what() << std::endl;
            return Result::FAILED;
        }
        std::vector<RalfPackageInfo> mountPkgList;
        auto status = lockPackage(package.value(), mountPkgList);
        if (status)
        {
            // We need to dump this to a temp file and add it as par of configMetadata
            auto tempFilePath = std::filesystem::temp_directory_path() / (packageId + "_" + version + "_metadata.json");
            if (serializeToJson(mountPkgList, tempFilePath))
            {
                std::cerr << "[libPackage] Successfully serialized mount package list to: " << tempFilePath << std::endl;
                configMetadata.ralfPkgPath = tempFilePath.string();
                unpackedPath = packagePath.parent_path().string();
                return Result::SUCCESS;
            }
        }
        return Result::FAILED;
    }

    Result RalfPackageImpl::Unlock(const std::string &packageId, const std::string &version)
    {
        return unmountDependentPackages(packageId, version) ? Result::SUCCESS : Result::FAILED;
    }

    Result RalfPackageImpl::GetFileMetadata(const std::string &fileLocator, std::string &packageId, std::string &version, ConfigMetaData &configMetadata)
    {
        auto packagePath = std::filesystem::path(fileLocator);
        auto package = ralf::Package::openWithoutVerification(packagePath);
        if (!package)
        {
            std::cerr << "[libPackage] Failed to open package : " << package.error().what() << std::endl;
            return Result::FAILED;
        }
        packageId = package->id();
        version = package->version().toString();
        configMetadata.appPath = packagePath.string();
        return Result::SUCCESS;
    }

    bool RalfPackageImpl::lockPackage(const ralf::Package &package, std::vector<RalfPackageInfo> &ralfMountInfo)
    {
        auto packageId = package.id();
        auto version = package.version().toString();
        std::cout << "[libPackage] Locking packages." << packageId << ", version " << version << std::endl;

        std::string pkgVerKey = packageId + "_" + version;
        // Check if already mounted
        if (mountedPackages.find(pkgVerKey) != mountedPackages.end())
        {
            // Increase mount count
            mountedPackages[pkgVerKey]->incMountCount();

            RalfPackageInfo ralfPkgInfo;
#ifdef ENABLE_LOCAL_MOUNT
            ralfPkgInfo.pkgMountPath = mountedPackages[pkgVerKey]->mountPath;
#else
            ralfPkgInfo.pkgMountPath = mountedPackages[pkgVerKey]->packageMount->mountPoint();
#endif
            ralfPkgInfo.pkgMetaDataPath = mountedPackages[pkgVerKey]->pkgJsonPath;
            ralfMountInfo.push_back(ralfPkgInfo);
            return true;
        }

        // So it is not mounted yet. Let us start with dependencies.

        auto pkgMetadata = package.metaData();
        if (!pkgMetadata)
        {
            std::cerr << "[libPackage] Failed to read package metadata for locking dependencies: " << pkgMetadata.error().what() << std::endl;
            return false;
        }

        auto dependencies = pkgMetadata->dependencies();
        for (const auto &dependency : dependencies)
        {
            std::string depPackageId = dependency.first;

            auto depPkgVersion = dependency.second;
            std::string depInstalledVersion;
            if (!identifyDependencyVersion(depPackageId, depPkgVersion, depInstalledVersion))
            {
                std::cerr << "[libPackage] Failed to identify dependency version for package: " << depPackageId << std::endl;
                return false;
            }
            auto depPackage = ralf::Package::openWithoutVerification(std::filesystem::path(AppInstallationPath) / depPackageId / depInstalledVersion / RalfPackage);

            if (!depPackage || !lockPackage(depPackage.value(), ralfMountInfo))
            {
                std::cerr << "[libPackage] Failed to lock dependent package: " << depPackageId << std::endl;
                return false;
            }
        }

        // Let us mount the package
        auto mountPath = std::filesystem::path(RDK_PACKAGE_MOUNT_PATH) / pkgVerKey / "rootfs";
        std::filesystem::create_directories(mountPath);
        std::cout << "[libPackage] Creating mount directory: " << mountPath << std::endl;
#ifdef ENABLE_LOCAL_MOUNT
        std::string erofsPath = getErofsBlobPath(packageId, version);

        std::cout << "[libPackage][LOCAL_MOUNT] Mounting " << erofsPath << std::endl;
        char loop_dev[20]; // to hold /dev/loopxxx
        if (setup_loop_device(erofsPath.c_str(), loop_dev, sizeof(loop_dev)) < 0)
        {
            std::cerr << "[libPackage][LOCAL_MOUNT] Unable to setup loop devices " << std::endl;
            return Result::FAILED;
        }
        std::cout << "[libPackage][LOCAL_MOUNT] Set up Loop device : " << loop_dev << std::endl;

        if (mount(loop_dev, mountPath.string().c_str(), "erofs", MS_RDONLY, NULL) < 0)
        {
            std::cerr << "[libPackage][LOCAL_MOUNT] Unable to mount loop device " << std::endl;
            detach_loop_device(loop_dev);
            return Result::FAILED;
        }
#else
        auto mountResult = package.mount(mountPath);
        if (!mountResult)
        {
            std::cerr << "[libPackage][RALFMOUNT] Failed to mount dependent package: " << packageId << mountResult.error().what() << std::endl;
            return false;
        }
#endif
        std::unique_ptr<MountedPackageInfo> mountInfo = std::make_unique<MountedPackageInfo>();

        auto configPath = std::filesystem::path(RDK_PACKAGE_MOUNT_PATH) / pkgVerKey / RDK_PACKAGE_CONFIG;
        if (dumpPackageInfo(package, configPath))
        {
            mountInfo->pkgJsonPath = configPath.string();
        }
#ifdef ENABLE_LOCAL_MOUNT
        mountInfo->loopDevice = loop_dev;
        mountInfo->mountPath = mountPath.string();
#else
        mountInfo->packageMount = std::make_unique<ralf::PackageMount>(std::move(mountResult.value()));
#endif

        mountedPackages[pkgVerKey] = std::move(mountInfo);

        RalfPackageInfo ralfPkgInfo;
        ralfPkgInfo.pkgMountPath = mountPath.string();
        ralfPkgInfo.pkgMetaDataPath = configPath.string();
        ralfMountInfo.push_back(ralfPkgInfo);

        return true;
    }
    bool RalfPackageImpl::unmountDependentPackages(const std::string &packageId, const std::string &version)
    {
        auto packagePath = std::filesystem::path(AppInstallationPath) / packageId / version / RalfPackage;
        std::cout << "[libPackage] Unlocking dependencies for package: " << packagePath.string() << std::endl;

        auto package = ralf::Package::openWithoutVerification(packagePath);
        if (!package)
        {
            std::cerr << "[libPackage] Failed to open" << packagePath.string() << " for unlocking dependencies: " << package.error().what() << std::endl;
            return false;
        }

        auto pkgMetadata = package->metaData();
        if (!pkgMetadata)
        {
            std::cerr << "[libPackage] Failed to read package metadata for unlocking dependencies: " << pkgMetadata.error().what() << std::endl;
            return false;
        }

        auto dependencies = pkgMetadata->dependencies();
        for (const auto &dependency : dependencies)
        {
            std::string depPackageId = dependency.first;
            ralf::VersionConstraint depPkgVersion = dependency.second;
            std::string depInstalledVersion;

            if (identifyDependencyVersion(depPackageId, depPkgVersion, depInstalledVersion))
            {
                if (!unmountDependentPackages(depPackageId, depInstalledVersion))
                {
                    std::cerr << "[libPackage] Failed to unmount dependent packages for package: " << depPackageId << ", version " << depInstalledVersion << std::endl;
                    // TODO revisit this logic
                    // return false;
                }
            }
            else
            {
                std::cerr << "[libPackage] Failed to idenitfy the version of dependency " << depPackageId << ", version " << depPkgVersion.toString() << std::endl;
            }
        }
        std::string depPackageKey = packageId + "_" + version;

        auto it = mountedPackages.find(depPackageKey);
        if (it == mountedPackages.end())
        {
            std::cerr << "[libPackage] Package not found in mounted packages: " << depPackageKey << std::endl;
            return false;
        }

#ifdef ENABLE_LOCAL_MOUNT

        it->second->decMountCount();
        if (it->second->mountCount == 0)
        {
            // Need to unmount the package
            umount(it->second->mountPath.c_str());
            // Detach the device
            detach_loop_device(it->second->loopDevice.c_str());
            mountedPackages.erase(it);
        }
#else
        if (it->second->packageMount->isMounted() == false)
        {
            std::cerr << "[libPackage][RALFMOUNT] Package is not mounted: " << depPackageKey << std::endl;
            return false;
        }
        it->second->decMountCount();
        if (it->second->mountCount == 0)
        {
            // Need to unmount the package
            it->second->packageMount->unmount();
            mountedPackages.erase(it);
        }
#endif // ENABLE_LOCAL_MOUNT
        return true;
    }

#ifdef ENABLE_LOCAL_MOUNT
    std::string RalfPackageImpl::getErofsBlobPath(const std::string &packageId, const std::string &version)
    {
        // We need to read package.erofs file as text and return the path
        std::filesystem::path erofsPath = std::filesystem::path(AppInstallationPath) / packageId / version / "package.erofs";

        std::ifstream erofsFile(erofsPath.string());
        if (!erofsFile.is_open())
        {
            std::cerr << "[libPackage][LOCAL_MOUNT] Failed to open package.erofs file: " << packageId << ", version " << version << std::endl;
            return "";
        }
        std::string blobPath;
        std::getline(erofsFile, blobPath);
        erofsFile.close();
        erofsPath = std::filesystem::path(AppInstallationPath) / packageId / version / blobPath;
        return erofsPath.string();
    }
#endif
    bool RalfPackageImpl::dumpPackageInfo(const ralf::Package &package, const std::filesystem::path &configPath)
    {
        // Case 1. It was once mounted, so no need to generate new one
        if (std::filesystem::exists(configPath))
        {
            return true;
        }
        // Case 2. Generate new one.
        auto packagejson = package.auxMetaDataFile(RDK_PACKAGE_CONFIG_MIME_TYPE);
        if (packagejson)
        {
            const auto contents = packagejson->readAll();
            if (contents)
            {
                std::ofstream outFile(configPath);
                if (!outFile.is_open())
                {
                    std::cerr << "[libPackage] Failed to open config file for writing: " << configPath << std::endl;
                    return false;
                }
                outFile.write(reinterpret_cast<const char *>(contents->data()), contents->size());
                outFile.close();
                return true;
            }
        }
        return false;
    }
    bool RalfPackageImpl::serializeToJson(const std::vector<RalfPackageInfo> &mountPkgList, const std::filesystem::path &outputPath) const
    {
        // Let us make use of nlohmann::json to get this done.
        /*
        The structure expected is as follows
        {
            "packages": [
                    {
                    "packagePath":"absolute path to package contents(mount point)",
                    "metadataPath":"absolute package config path (in json format"
                    },
                    {
                    "packagePath":"absolute path to package contents(mount point)",
                    "metadataPath":"absolute package config path (in json format"      },
                    {
                    "packagePath":"absolute path to package contents(mount point)",
                    "metadataPath":"absolute package config path (in json format"
                    }
            ]
        }
        */

        Json::Value packages(Json::arrayValue);
        for (const auto &pkgInfo : mountPkgList)
        {
            Json::Value pkgJson;
            pkgJson["pkgMountPath"] = pkgInfo.pkgMountPath;
            pkgJson["pkgMetaDataPath"] = pkgInfo.pkgMetaDataPath;
            packages.append(pkgJson);
        }
        Json::Value root;
        root["packages"] = packages;

        std::ofstream outputFile(outputPath);
        if (!outputFile.is_open())
        {
            std::cerr << "[libPackage] Failed to open output file: " << outputPath << std::endl;
            return false;
        }
        outputFile << root.toStyledString();
        outputFile.close();
        return true;
    }
} // namespace packagemanager
