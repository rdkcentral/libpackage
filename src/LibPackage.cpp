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

#include "LibPackage.h"

namespace packagemanager
{

    uint32_t LibPackage::Install(const std::string &packageId, const std::string &version, const std::string &fileLocator)
    {
        return 0;
    }

    uint32_t LibPackage::Uninstall(const std::string &packageId)
    {
        return 0;
    }

    uint32_t LibPackage::GetList(std::string &packageList)
    {
        return 0;
    }

    uint32_t LibPackage::Lock(const std::string &packageId, const std::string &version, std::string &unpackedPath)
    {
        return 0;
    }

    uint32_t LibPackage::Unlock(const std::string &packageId, const std::string &version)
    {
        return 0;
    }

    uint32_t LibPackage::GetLockInfo(const std::string &packageId, const std::string &version, std::string &unpackedPath, bool &locked)
    {
        return 0;
    }

}

