/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 Liberty Global Service B.V.
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
#include <iostream>

namespace packagemanager
{

    class LibPackage : public IPackageImpl
    {
    public:
        ~LibPackage() override = default;

        uint32_t Install(const std::string &packageId, const std::string &version, const std::string &fileLocator) override;
        uint32_t Uninstall(const std::string &packageId) override;

        uint32_t GetList(std::string &packageList) override;

        uint32_t Lock(const std::string &packageId, const std::string &version, std::string &unpackedPath) override;
        uint32_t Unlock(const std::string &packageId, const std::string &version) override;

        uint32_t GetLockInfo(const std::string &packageId, const std::string &version, std::string &unpackedPath, bool &locked) override;

        //static std::shared_ptr<packagemanager::IPackageImpl> instance();

    private:
        LibPackage() = default;
        static std::shared_ptr<packagemanager::IPackageImpl> instance;
    };

} // namespace packagemanager

