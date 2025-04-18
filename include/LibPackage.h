/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
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

namespace packagemanager
{

    class LibPackage : public IPackageImpl
    {
    public:
        ~LibPackage() override = default;

    Result Initialize(ConfigMetadataArray &configMetadata) override;

    Result Install(const std::string &packageId, const std::string &version, const std::string &fileLocator, ConfigMetaData &configMetadata) override;
    Result Uninstall(const std::string &packageId) override;

    Result GetList(std::string &packageList) override;

    Result Lock(const std::string &packageId, const std::string &version, std::string &unpackedPath, ConfigMetaData &configMetadata) override;
    Result Unlock(const std::string &packageId, const std::string &version) override;

    Result GetLockInfo(const std::string &packageId, const std::string &version, std::string &unpackedPath, bool &locked) override;

    static std::shared_ptr<packagemanager::IPackageImpl> instance();

    private:
        LibPackage() = default;
    };

}

