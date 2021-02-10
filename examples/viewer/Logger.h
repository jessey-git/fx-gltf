// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <iostream>
#include "StringFormatter.h"

namespace Logger
{
    template <typename... Args>
    static void Write(std::basic_string_view<char> format, Args &&... args)
    {
        if constexpr (sizeof...(args) > 0)
        {
            std::cout << fx::common::StringFormatter::Format(format, std::forward<Args>(args)...);
        }
        else
        {
            std::cout << format;
        }
    }

    template <typename... Args>
    static void WriteLine(std::basic_string_view<char> format, Args &&... args)
    {
        Logger::Write(format, std::forward<Args>(args)...);
        std::cout << std::endl;
    }
} // namespace Logger
