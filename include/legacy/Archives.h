/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2025 RDK Management
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

#include <string>
#include <stdexcept>

namespace packagemanager
{
    namespace Archive
    {
        /**
         * Given a compressed archive in tar.gz format, this function will extract the content to the destinationPath
         * @param archivePath Full path of the archive
         * @param destinationPath Full path to the destination directory
         * @return int  1 if the extraction succeeeds, 0 otherwise
         */
        int unpackArchive(const std::string &filePath, const std::string &destinationDir);
    } // namespace Archive
} // namespace packagemanager
