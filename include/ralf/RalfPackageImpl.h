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

        std::vector<std::unique_ptr<ConfigMetadataKey> > mInstalledPackages;

        bool lockPackage(const ralf::Package &package, std::vector< RalfPackageInfo> &ralfMountInfo);
        bool unmountDependentPackages(const std::string &packageId, const std::string &version);
        bool identifyDependencyVersion(const std::string &depPackageId, const ralf::VersionConstraint &depPackageVersion, std::string &depInstalledVersion);
        bool dumpPackageInfo(const ralf::Package &package, const std::filesystem::path &configPath);
        bool serializeToJson(const std::vector<RalfPackageInfo> &mountPkgList, const std::filesystem::path &outputPath) const;
#ifdef ENABLE_LOCAL_MOUNT
        std::string getErofsBlobPath(const std::string &packageId, const std::string &version);
#endif
    };

}
