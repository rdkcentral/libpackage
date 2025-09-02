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

#include <gtest/gtest.h>
#include "PackageImpl.h"
#include "IPackageImpl.h"
#include <gmock/gmock.h>
#include <sqlite3.h>

class PackageImplTest : public ::testing::Test {
protected:
    packagemanager::PackageImpl packageImpl;
};

TEST_F(PackageImplTest, InstallHandlesFailed) {
    std::string packageId = "test_package";
    std::string version = "1.0.0";
    packagemanager::NameValues additionalMetadata = {};
    std::string fileLocator = "http://com.rdk/cobalt";
    packagemanager::ConfigMetaData configMetadata;

    EXPECT_EQ(packageImpl.Install(packageId, version, additionalMetadata, fileLocator, configMetadata), packagemanager::Result::FAILED);
    EXPECT_EQ(packageImpl.Install("", version, additionalMetadata, fileLocator, configMetadata), packagemanager::Result::FAILED);
    EXPECT_EQ(packageImpl.Install(packageId, "", additionalMetadata, fileLocator, configMetadata), packagemanager::Result::FAILED);
    EXPECT_EQ(packageImpl.Install(packageId, version, additionalMetadata, "", configMetadata), packagemanager::Result::FAILED);

}

TEST_F(PackageImplTest, DISABLED_InitializeHandlesEmptyConfig) {
    std::string configStr = "";
    packagemanager::ConfigMetadataArray configMetadata;

    packagemanager::Result result = packageImpl.Initialize(configStr, configMetadata);

    EXPECT_EQ(result, packagemanager::Result::FAILED);
}

TEST_F(PackageImplTest, DISABLED_InitializeHandlesInvalidConfig) {
    std::string configStr = "invalid_config";
    packagemanager::ConfigMetadataArray configMetadata;

    packagemanager::Result result = packageImpl.Initialize(configStr, configMetadata);

    EXPECT_EQ(result, packagemanager::Result::FAILED);
}

TEST_F(PackageImplTest, InitializeHandlesValidConfig) {
    std::string configStr = R"({"appspath":"/opt/dac_apps/apps","dbpath":"/opt/dac_apps","datapath":"/opt/dac_apps/data","annotationsFile":"config.json","annotationsRegex":"public\\.*","downloadRetryAfterSeconds":30,"downloadRetryMaxTimes":4,"downloadTimeoutSeconds":900})";
    packagemanager::ConfigMetadataArray configMetadata;

    packagemanager::Result result = packageImpl.Initialize(configStr, configMetadata);

    EXPECT_EQ(result, packagemanager::Result::SUCCESS);
}


TEST_F(PackageImplTest, DISABLED_UninstallHandlesNullPackageId) {
    std::string emptyPackageId = "";
    auto result = packageImpl.Uninstall(emptyPackageId);
    EXPECT_EQ(result, packagemanager::Result::FAILED);
}

TEST_F(PackageImplTest, LockHandlesInvalidPackageId) {
    std::string invalidPackageId = "";
    std::string version = "1.0.0";
    std::string unpackedPath;
    packagemanager::ConfigMetaData configMetadata;
    auto result = packageImpl.Lock(invalidPackageId, version, unpackedPath, configMetadata);
    EXPECT_EQ(result, packagemanager::Result::FAILED);
}

TEST_F(PackageImplTest, DISABLED_UnlockHandlesEmptyPackageIdAndVersion) {
    std::string PackageId = "Test";
    std::string Version = "1.0.0";
    EXPECT_EQ(packageImpl.Unlock("",Version), packagemanager::Result::FAILED);
    EXPECT_EQ(packageImpl.Unlock(PackageId,""), packagemanager::Result::FAILED);
}

TEST_F(PackageImplTest, ValidDataTesting) {
    std::string configStr = R"({"appspath":"/opt/dac_apps/apps","dbpath":"/opt/dac_apps","datapath":"/opt/dac_apps/data","annotationsFile":"config.json","annotationsRegex":"public\\.*","downloadRetryAfterSeconds":30,"downloadRetryMaxTimes":4,"downloadTimeoutSeconds":900})";
    packagemanager::ConfigMetadataArray configMetadata;

    packagemanager::Result result1 = packageImpl.Initialize(configStr, configMetadata);

    EXPECT_EQ(result1, packagemanager::Result::SUCCESS);
    std::string dbPath = "/opt/dac_apps/0/apps.db";
    sqlite3* db;
    ASSERT_EQ(sqlite3_open(dbPath.c_str(), &db), SQLITE_OK);
    const char* insertDataQuery = R"(
        INSERT INTO installed_apps (app_idx, version, name, category, url, app_path, created, resources, metadata)
        VALUES ('1', '1.0', 'testapp', 'category', 'http://192.168.0.178/com.rdk.cobalt.kirkstone_thunder_4.4.tar.gz',
                '0/com.rdk.sleepy/1.0/', 'Wed Apr 30 14:05:16 2025', NULL, NULL);
    )";
    ASSERT_EQ(sqlite3_exec(db, insertDataQuery, nullptr, nullptr, nullptr), SQLITE_OK);
    const char* insertQuery = R"(
        INSERT INTO apps (type, app_id, data_path, created)
        VALUES ('dac', 'com.rdk.sleepy', '0/com.rdk.sleepy/', 'Wed Apr 30 14:05:16 2025');
    )";
    ASSERT_EQ(sqlite3_exec(db, insertQuery, nullptr, nullptr, nullptr), SQLITE_OK);
    std::string packageList;
    //auto result = packageImpl.GetList(packageList);
    //EXPECT_EQ(result, packagemanager::Result::SUCCESS);
    //EXPECT_FALSE(packageList.empty());
    std::string pID = "com.rdk.sleepy";
    std::string ver = "1.0";
    std::string unpackedPath;
    packagemanager::ConfigMetaData confMetadata;
    //result = packageImpl.Lock(pID,ver,unpackedPath,confMetadata);
    packagemanager::NameValues additionalLocks;
    auto result = packageImpl.Lock(pID,ver,unpackedPath,confMetadata,additionalLocks);
    EXPECT_EQ(result, packagemanager::Result::SUCCESS);
    //result = packageImpl.Unlock(pID,ver);
    //EXPECT_EQ(result, packagemanager::Result::SUCCESS);
    result = packageImpl.Uninstall(pID);
    EXPECT_EQ(result, packagemanager::Result::SUCCESS);
    sqlite3_close(db);
    remove(dbPath.c_str());
}
