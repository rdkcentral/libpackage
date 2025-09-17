/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 *  Copyright 2025 RDK Management
 *  Copyright 2021 Liberty Global Service B.V.
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

#include "Archives.h"
#include "Debug.h"

#include <archive.h>
#include <archive_entry.h>

#include <memory>

namespace packagemanager
{
    namespace Archive
    {
        static constexpr int BLOCK_SIZE = 10240;
        static constexpr int ARCHIVE_FLAGS = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;

        int unpackArchive(const std::string &archivePath, const std::string &destinationPath)
        {
            int result = 0;
            struct archive *theArchive = archive_read_new();
            archive_read_support_format_tar(theArchive);
            archive_read_support_filter_gzip(theArchive);

            // Read the archive
            if (archive_read_open_filename(theArchive, archivePath.c_str(), BLOCK_SIZE) != ARCHIVE_OK)
            {
                ERROR("Failed to open archive: ", archive_error_string(theArchive));
                return result;
            }
            DEBUG("Archive opened successfully ", archivePath.c_str());

            // Extract the contents
            struct archive_entry *entry{};
            while (true)
            {
                auto readHeaderResult = archive_read_next_header(theArchive, &entry);

                if (readHeaderResult == ARCHIVE_EOF)
                {
                    DEBUG("archive read successfully");
                    result = 1;
                    break;
                }
                else if (readHeaderResult != ARCHIVE_OK && readHeaderResult != ARCHIVE_WARN)
                {
                    ERROR("error while reading entry  ", archive_error_string(theArchive));
                }
                else if (readHeaderResult == ARCHIVE_WARN)
                {
                    WARNING("Warning while reading entry ", archive_error_string(theArchive));
                }

                std::string destPath{destinationPath + archive_entry_pathname(entry)};
                archive_entry_set_pathname(entry, destPath.c_str());

                const char *origHardlink = archive_entry_hardlink(entry);
                if (origHardlink)
                {
                    std::string destPathHardLink{destinationPath + '/' + origHardlink};
                    archive_entry_set_hardlink(entry, destPathHardLink.c_str());
                }

                auto extractStatus = archive_read_extract(theArchive, entry, ARCHIVE_FLAGS);
                if (extractStatus == ARCHIVE_OK || extractStatus == ARCHIVE_WARN)
                {
                    DEBUG("extracted: %s", archive_entry_pathname(entry));
                    if (extractStatus == ARCHIVE_WARN)
                    {
                        WARNING("Warning while extracting ", archive_error_string(theArchive));
                    }
                }
                else
                {
                    ERROR("Error while extracting ", archive_error_string(theArchive));
                }
            }
            //Clean up any open handles
            archive_read_close(theArchive);
            archive_read_free(theArchive);
            return result;
        }

    } // namespace Archive
} // namespace packagemanager