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
#include <iostream>
namespace packagemanager
{
    enum RETURN_CODE
    {
        RETURN_SUCCESS = 0,
        RETURN_ERROR = 1
    };
    enum LOG_LEVEL
    {
        LOG_LEVEL_ERROR = 0,
        LOG_LEVEL_WARNING,
        LOG_LEVEL_INFO,
        LOG_LEVEL_DEBUG
    };

    template <typename... Args>
    void log(LOG_LEVEL logLevel, Args &&...args)
    {
        // By default error and warning messages are printed
        if (logLevel == LOG_LEVEL_DEBUG)
        {
#ifndef DEBUG_ENABLED
            return; //Enable only if debugging is enabled
#endif
        }
        if (logLevel == LOG_LEVEL_ERROR || logLevel == LOG_LEVEL_WARNING)
        {
            //Sent it to error output
            std::cerr << "[Libpackage] ";
            (std::cerr << ... << args) << std::endl; // C++17 fold expression
        }
        else
        {

            std::cout << "[Libpackage] ";
            (std::cout << ... << args) << std::endl; // C++17 fold expression
        }
    }
#define DEBUG(...) log(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define INFO(...) log(LOG_LEVEL_INFO, __VA_ARGS__)
#define WARNING(...) log(LOG_LEVEL_WARNING, __VA_ARGS__)
#define ERROR(...) log(LOG_LEVEL_ERROR, __VA_ARGS__)
//#define LOG(...) log(LOG_LEVEL_INFO, __VA_ARGS__)
}