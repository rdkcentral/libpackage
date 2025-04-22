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

#include <string>
#include <memory>

namespace packagemanager
{

    enum Result : uint8_t
    {
        SUCCESS,
        FAILED
    };

    class IPackageImpl
    {
    public:
        virtual ~IPackageImpl() = default;

        virtual Result Install(const std::string &packageId, const std::string &version, const std::string &fileLocator) = 0;
        virtual Result Uninstall(const std::string &packageId) = 0;

        virtual Result GetList(std::string &packageList) = 0;

        virtual Result Lock(const std::string &packageId, const std::string &version, std::string &unpackedPath) = 0;
        virtual Result Unlock(const std::string &packageId, const std::string &version) = 0;

        virtual Result GetLockInfo(const std::string &packageId, const std::string &version, std::string &unpackedPath, bool &locked) = 0;

        static std::shared_ptr<packagemanager::IPackageImpl> instance();
    };

}
