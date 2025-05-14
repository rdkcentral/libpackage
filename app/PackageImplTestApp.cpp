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

#include "IPackageImpl.h"
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>

const std::string VERSION = "1.0.5";
int testGetList(std::shared_ptr<packagemanager::IPackageImpl> packageImpl, std::string &packageList)
{

    std::cout << "Testing GetList method..." << std::endl;
    if (packageImpl->GetList(packageList) == packagemanager::Result::SUCCESS)
    {
        std::cout << "Package List: " << packageList << std::endl;
        return 1; // Success
    }
    else
    {
        std::cout << "Failed to retrieve package list." << std::endl;
        return 0; // Failure
    }
}
void testUninstall(std::shared_ptr<packagemanager::IPackageImpl> packageImpl, std::string &packageId)
{
    std::cout << "Testing Uninstall method for package: " << packageId << std::endl;
    if (packageImpl->Uninstall(packageId) == packagemanager::Result::SUCCESS)
    {
        std::cout << "Successfully uninstalled package: " << packageId << std::endl;
    }
    else
    {
        std::cout << "Failed to uninstall package: " << packageId << std::endl;
    }
}
bool testLock(std::shared_ptr<packagemanager::IPackageImpl> packageImpl, std::string &packageId)
{
    std::cout << "Testing Lock method for package: " << packageId << std::endl;
    std::string unpackedPath;
    packagemanager::ConfigMetaData configMetadata;
    if (packageImpl->Lock(packageId, "", unpackedPath, configMetadata) == packagemanager::Result::SUCCESS)
    {
        std::cout << "Successfully locked package: " << packageId << ", Unpacked Path: " << unpackedPath << std::endl;
        return true; // Success
    }
    else
    {
        std::cout << "Failed to lock package: " << packageId << std::endl;
        return false; // Failure
    }
}
bool testUnLock(std::shared_ptr<packagemanager::IPackageImpl> packageImpl, std::string &packageId, std::string &version)
{
    std::cout << "Testing Unlock method for package: " << packageId << std::endl;
    if (packageImpl->Unlock(packageId, version) == packagemanager::Result::SUCCESS)
    {
        std::cout << "Successfully unlocked package: " << packageId << std::endl;
        return true; // Success
    }
    else
    {
        std::cout << "Failed to unlock package: " << packageId << std::endl;
        return false; // Failure
    }
}
bool testInstall(std::shared_ptr<packagemanager::IPackageImpl> packageImpl)
{
    std::cout << "Testing Install method..." << std::endl;
    // Install a package. This is downloaded from DAC app store and placed in the specified location.
    std::string fileUrl = "/opt/downloads/com.rdk.app.cobalt2024-1.0.0-rpi4-1.0.0-b34e9a38a2675d4cd02cf89f7fc72874a4c99eb0-dbg.tar.gz";
    std::string packageId = "com.rdk.app.cobalt2024";
    std::string version = "1.0.0";
    std::string appName = "YouTube 2024";
    std::string category = "Media";
    std::string type = "application/dac.native";

    packagemanager::NameValues additionalMetadata;
    additionalMetadata.push_back(std::make_pair("type", type));
    additionalMetadata.push_back(std::make_pair("appName", appName));
    additionalMetadata.push_back(std::make_pair("category", category));
    packagemanager::ConfigMetaData configMetadata;

    if (packageImpl->Install(packageId, version, additionalMetadata, fileUrl, configMetadata) == packagemanager::Result::SUCCESS)
    {
        std::cout << "Successfully installed package: " << packageId << std::endl;
        return true; // Success
    }
    else
    {
        std::cout << "Failed to install package: " << packageId << std::endl;
        return false; // Failure
    }
}
int main(int argc, char *argv[])
{
    std::cout << "PackageImplTestApp version " << VERSION << std::endl;

    std::shared_ptr<packagemanager::IPackageImpl> packageImpl = packagemanager::IPackageImpl::instance();
    std::string packageList;
    packagemanager::ConfigMetadataArray configMetadataArray;
    std::string configString = R"({"appspath":"/opt/dac_apps/apps","dbpath":"/opt/dac_apps","datapath":"/opt/dac_apps/data","annotationsFile":"config.json","annotationsRegex":"public\\.*","downloadRetryAfterSeconds":30,"downloadRetryMaxTimes":4,"downloadTimeoutSeconds":900})";
    if (packageImpl->Initialize(configString, configMetadataArray) != packagemanager::SUCCESS)
    {
        std::cout << "Failed to initialize package manager." << std::endl;
        return 1;
    }
    std::cout << "Package Manager Initialized." << std::endl;

    testInstall(packageImpl);

    if (testGetList(packageImpl, packageList))
    {
        // Get the first package ID from the list
        std::istringstream iss(packageList);
        std::string errs;
        boost::property_tree::ptree packages;
        boost::property_tree::read_json(iss, packages);

        auto firstItem = packages.begin();
        if (firstItem == packages.end())
        {
            std::cout << "No packages found in the list." << std::endl;
            return 1;
        }
        std::string packageId = firstItem->second.get<std::string>("packageId");
        std::string version = firstItem->second.get<std::string>("version", "1.0.0"); // Default to "1.0.0" if not present
        std::cout << "First package ID: " << packageId << " Version: " << version << std::endl;
        if (testLock(packageImpl, packageId))
        {
            // Test Unlock method
            testUnLock(packageImpl, packageId, version);
        }
        testUninstall(packageImpl, packageId);
    }

    std::cout << "Test completed." << std::endl;
    return 0;
}
