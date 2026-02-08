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

#pragma once
#include <map>
#include <vector>
#include <utility>
#include <memory>
#include <IPackageImpl.h>
#include <ralf/Package.h>
#include <ralf/VersionConstraint.h>

#ifndef DAC_APP_PATH
#define DAC_APP_PATH "/opt/media/apps/"
#endif

#ifndef RDK_PACKAGE_CERT_PATH
#define RDK_PACKAGE_CERT_PATH "/etc/rdk/certs"
#endif

#define RDK_PACKAGE_CONFIG_MIME_TYPE "application/vnd.rdk.package.config.v1+json"
#define RDK_PACKAGE_MOUNT_PATH "/tmp/mounts/"
#define RDK_PACKAGE_CONFIG "config.json"
namespace ralf = LIBRALF_NS;

namespace packagemanager
{
    struct MountedPackageInfo
    {
        // We need this to keep track of how many times a package is mounted. Otherwise we will unmount it too early
        int mountCount = 1;
        std::string pkgJsonPath;
        std::unique_ptr<ralf::PackageMount> packageMount;
        void incMountCount() { mountCount++; }
        void decMountCount() { mountCount--; }
    };

    typedef struct _RalfPackageInfo
    {
        std::string pkgMountPath;
        std::string pkgMetaDataPath;
    } RalfPackageInfo;

    std::map<std::string, std::unique_ptr<MountedPackageInfo> > mountedPackages;

    class RalfPackageImpl : public IPackageImpl
    {
    private:
        static int getInstalledPackages(std::vector<std::string> &pacakgeList);
        static void getPackageIdAndVersionFromRalfPackage(const std::string &packagePath, std::string &appId, std::string &appVersion);

    public:
        ~RalfPackageImpl() override = default;
        RalfPackageImpl() {}

        Result Initialize(const std::string &configStr, ConfigMetadataArray &aConfigMetadata) override;

        Result Install(const std::string &packageId, const std::string &version, const NameValues &additionalMetadata, const std::string &fileLocator, ConfigMetaData &configMetadata) override;
        Result Uninstall(const std::string &packageId) override;

        Result Lock(const std::string &packageId, const std::string &version, std::string &unpackedPath, ConfigMetaData &configMetadata, NameValues &additionalLocks) override;
        Result Unlock(const std::string &packageId, const std::string &version) override;
        Result GetFileMetadata(const std::string &fileLocator, std::string &packageId, std::string &version, ConfigMetaData &configMetadata) override;

    private:
        static std::string RalfPackage;
        static std::string AppInstallationPath;
        static std::string pkgCertDirPath;

        // Flag to check initialisation status
        bool mIsInitialized = false;

        // For package verification
        ralf::VerificationBundle mVerificationBundle;

        std::vector<std::unique_ptr<ConfigMetadataKey> > mInstalledPackages;

        /**
         * Initializes the verification bundle by loading certificates from the specified directory.
         * @return true if at least one certificate was successfully loaded; false otherwise.
         */
        bool initializeVerificationBundle();

        /**
         * Opens a package file and returns a Result containing the Package object.
         * Sets the passedVerification flag to true if the package verification is successful; false otherwise.
         * @param packageFile The path to the package file.
         * @param passedVerification Output parameter to indicate if the package verification was successful.
         * @return A Result containing the Package object if successful; an error otherwise.
         */
        ralf::Result<ralf::Package> openPackage(const std::string &packageFile, bool &passedVerification);

        /**
         * Locks the specified package for exclusive access. The package is verified, dependent packages are mounted,
         * and necessary resources are allocated.
         * @param package The package to be locked.
         * @param ralfMountInfo Output parameter to hold information about the mounted package.
         * @return true if the package is locked successfully; false otherwise.
         */
        bool lockPackage(const ralf::Package &package, std::vector<RalfPackageInfo> &ralfMountInfo);

        /**
         * Mounts the dependent packages required by the specified package.
         * @param package The package whose dependencies are to be unmounted.
         * @return true if all dependent packages are unmounted successfully; false otherwise.
         */
        bool unmountDependentPackages(const ralf::Package &package);

        /**
         * Identifies the installed version of a dependent package that satisfies the given version constraint.
         * @param depPackageId The ID of the dependent package.
         * @param depPackageVersion The version constraint for the dependent package.
         * @param depInstalledVersion Output parameter to hold the identified installed version.
         * @return true if a suitable installed version is found; false otherwise.
         */
        bool identifyDependencyVersion(const std::string &depPackageId, const ralf::VersionConstraint &depPackageVersion, std::string &depInstalledVersion);

        /**
         * Dumps the package information to a specified configuration path.
         * @param package The package whose information is to be dumped.
         * @param configPath The path where the package information will be dumped.
         * @return true if the package information is dumped successfully; false otherwise.
         */
        bool dumpPackageInfo(const ralf::Package &package, const std::filesystem::path &configPath);

        /**
         * Serializes the list of mounted packages to a JSON file at the specified output path.
         * @param mountPkgList The list of mounted packages to be serialized.
         * @param outputPath The path where the JSON file will be created.
         * @return true if serialization is successful; false otherwise.
         */
        bool serializeToJson(const std::vector<RalfPackageInfo> &mountPkgList, const std::filesystem::path &outputPath) const;
    };

}
