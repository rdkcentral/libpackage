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

#include <string>

template <typename T>
inline T retrieveInputFromUser(const std::string &prompt, bool allowEmpty, T defaultValue)
{
    T value = defaultValue;

    std::string input;

    std::cout << prompt << "Default (" << defaultValue << "): ";
    std::getline(std::cin, input);

    if (input.empty() && allowEmpty)
    {
        return defaultValue;
    }

    while (true)
    {
        std::istringstream iss(input);
        if (!(iss >> value))
        {
            std::cout << "Invalid input. Please try again: ";
            std::getline(std::cin, input);
            iss.clear();
            iss.str(input);
            continue;
        }
        else
            break;
    }
    return value;
}

void testPackageInfo(std::shared_ptr<packagemanager::IPackageImpl> packageImpl)
{
    std::string fileLocator = retrieveInputFromUser<std::string>("Enter the file locator: ", false, "");
    std::string packageId = retrieveInputFromUser<std::string>("Enter the package ID: ", true, "");
    std::string version = retrieveInputFromUser<std::string>("Enter the version: ", true, "");
    packagemanager::ConfigMetaData configMetadata;

    if (packageImpl->GetFileMetadata(fileLocator, packageId, version, configMetadata) == packagemanager::Result::SUCCESS)
    {
        std::cout << "Successfully retrieved package metadata." << std::endl;
        std::cout << "Package ID: " << packageId << std::endl;
        std::cout << "Version: " << version << std::endl;
        std::cout << "File Locator: " << fileLocator << std::endl;
        std::cout << "App installed path: " << configMetadata.appPath << std::endl;
    }
    else
    {
        std::cout << "Failed to retrieve package metadata." << std::endl;
    }
}
void testUninstall(std::shared_ptr<packagemanager::IPackageImpl> packageImpl)
{
    std::string packageId = retrieveInputFromUser<std::string>("Enter the package ID to uninstall: ", false, "");

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
bool testLock(std::shared_ptr<packagemanager::IPackageImpl> packageImpl)
{
    std::string packageId = retrieveInputFromUser<std::string>("Enter the package ID to lock: ", false, "");

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
bool testUnLock(std::shared_ptr<packagemanager::IPackageImpl> packageImpl)
{
    std::string packageId = retrieveInputFromUser<std::string>("Enter the package ID to unlock: ", false, "");
    std::string version = retrieveInputFromUser<std::string>("Enter the version to unlock: ", false, "");

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
    std::string packageId = retrieveInputFromUser<std::string>("Enter the package ID to install: ", false, "");
    std::string version = retrieveInputFromUser<std::string>("Enter the version to install: ", false, "");
    // Install a package. This is downloaded from DAC app store and placed in the specified location.
    std::string fileUrl = retrieveInputFromUser<std::string>("Enter the package location to install: ", false, "");

    packagemanager::NameValues additionalMetadata;
    packagemanager::ConfigMetaData configMetadata;

    if (packageImpl->Install(packageId, version, additionalMetadata, fileUrl, configMetadata) == packagemanager::Result::SUCCESS)
    {
        std::cout << "Successfully installed package: " << packageId << std::endl;
        std::cout << "App installed path: " << configMetadata.appPath << std::endl;
        return true; // Success
    }
    else
    {
        std::cout << "Failed to install package: " << packageId << std::endl;
        return false; // Failure
    }
}

void printMainMenu()
{
    std::cout << "--------------------------" << std::endl;
    std::cout << "LibPackage Test Utility" << std::endl;
    std::cout << "Select an option:" << std::endl;

    std::cout << "1. Install a package" << std::endl;
    std::cout << "2. Lock a package" << std::endl;
    std::cout << "3. Get package information" << std::endl;
    std::cout << "4. UnLock a package" << std::endl;
    std::cout << "5. Uninstall a package" << std::endl;
    std::cout << "0 . Exit" << std::endl;
    std::cout << "--------------------------" << std::endl;
}
int processUserCommands(std::shared_ptr<packagemanager::IPackageImpl> packageImpl)
{

    while (true)
    {
        printMainMenu();

        int choice = retrieveInputFromUser<int>("Enter your choice: ", false, 0);
        switch (choice)
        {
        case 1:
            std::cout << "Testing install method..." << std::endl;
            testInstall(packageImpl);
            break;
        case 2:
            testLock(packageImpl);
            break;
        case 3:
            testPackageInfo(packageImpl);
            break;
        case 4:
            testUnLock(packageImpl);
            break;
        case 5:
            testUninstall(packageImpl);
            break;
        case 0:
            std::cout << "Exiting..." << std::endl;
            return 0;
        default:
            std::cout << "Invalid choice. Please try again." << std::endl;
        }
    }
    return 0;
}
void dumpCurrentPackages(const packagemanager::ConfigMetadataArray &configMetadataMap)
{
    std::cout << "Current Packages:" << std::endl;
    for (const auto &configMetadata : configMetadataMap)
    {
        const std::pair<std::string, std::string> & appKey = configMetadata.first;
        std::cout << "Package ID: " << appKey.first << ", Version: " << appKey.second << std::endl;
        const packagemanager::ConfigMetaData & configData = configMetadata.second;
        std::cout << "Ralf path: " << configData.ralfPkgPath << ", App path: " << configData.appPath << std::endl;
    }
}
int main(int argc, char *argv[])
{
    std::cout << "PackageImplTestApp version 1.2.0" << std::endl;

    std::shared_ptr<packagemanager::IPackageImpl> packageImpl = packagemanager::IPackageImpl::instance();
    std::string packageList;
    packagemanager::ConfigMetadataArray configMetadataMap;
    std::string configString = R"({"appspath":"/opt/dac_apps/apps","dbpath":"/opt/dac_apps","datapath":"/opt/dac_apps/data","annotationsFile":"config.json","annotationsRegex":"public\\.*"})";
    if (packageImpl->Initialize(configString, configMetadataMap) != packagemanager::Result::SUCCESS)
    {
        std::cout << "Failed to initialize package manager." << std::endl;
        return 1;
    }
    std::cout << "Package Manager Initialized. " << std::endl;
    dumpCurrentPackages(configMetadataMap);
    processUserCommands(packageImpl);

    std::cout << "Test completed." << std::endl;
    return 0;
}
