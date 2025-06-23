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

#include "IPackageImpl.h"
#include "Executor.h"
namespace packagemanager
{

    class PackageImpl : public IPackageImpl
    {
    public:
        ~PackageImpl() override = default;
        PackageImpl() {}

        Result Initialize(const std::string &configStr, ConfigMetadataArray &configMetadata) override;

        Result Install(const std::string &packageId, const std::string &version, const NameValues &additionalMetadata, const std::string &fileLocator, ConfigMetaData &configMetadata) override;

        Result Lock(const std::string &packageId, const std::string &version, std::string &unpackedPath, ConfigMetaData &configMetadata) override;
        Result Unlock(const std::string &packageId, const std::string &version) override;

        Result Uninstall(const std::string &packageId) override;

    private:
        packagemanager::Executor executor;
        bool populateConfigValues(const std::string &configjsonfile, ConfigMetaData &configMetadata /* out*/);
    };

}
