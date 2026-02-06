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
    int setup_loop_device(const char *img_path, char *loop_dev, size_t loop_dev_size);
    void detach_loop_device(const char *loop_dev);

#ifdef ENABLE_LOCAL_MOUNT
    struct MountedPackageInfo
    {
        int mountCount = 1;
        std::string loopDevice;
        std::string mountPath;
        std::string pkgJsonPath;
        void incMountCount() { mountCount++; }
        void decMountCount() { mountCount--; }
    };
#else
    struct MountedPackageInfo
    {
        // We need this to keep track of how many times a package is mounted. Otherwise we will unmount it too early
        int mountCount = 1;
        std::string pkgJsonPath;
        std::unique_ptr<ralf::PackageMount> packageMount;
        void incMountCount() { mountCount++; }
        void decMountCount() { mountCount--; }
    };
#endif

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

        // For package verification
        ralf::VerificationBundle mVerificationBundle;

        std::vector<std::unique_ptr<ConfigMetadataKey> > mInstalledPackages;

        /**
         * Initializes the verification bundle by loading certificates from the specified directory.
         * @return true if at least one certificate was successfully loaded; false otherwise.
         */
        bool initilizeVerificationBundle();

        /**
         * Verifies the package's signature using the initialized verification bundle.
         * @param package The package to be verified.
         * @return Result::SUCCESS if the package is verified successfully; Result::FAILED otherwise.
         */
        bool lockPackage(const ralf::Package &package, std::vector<RalfPackageInfo> &ralfMountInfo);

        /**
         * Mounts the dependent packages required by the specified package.
         * @param packageId The ID of the package whose dependencies are to be mounted.
         * @param version The version of the package whose dependencies are to be mounted.
         * @return true if all dependent packages are mounted successfully; false otherwise.
         */
        bool unmountDependentPackages(const std::string &packageId, const std::string &version);

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
#ifdef ENABLE_LOCAL_MOUNT
        ` /**
           * Constructs the EROFS blob path for the specified package ID and version.
           * The path is read from the package.erofs file located in the package directory.
           *
           * @param packageId The ID of the package.
           * @param version The version of the package.
           * @return The constructed EROFS blob path as a string.
           */
            std::string getErofsBlobPath(const std::string &packageId, const std::string &version);
#endif
    };

}
