// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <cstdio>
#include <exception>
#include <Platform/platform.h>

namespace COMUtil
{
    // Helper class for COM exceptions
    class com_exception : public std::exception
    {
    public:
        explicit com_exception(HRESULT hr) noexcept
            : result(hr) {}

        const char * what() const noexcept override
        {
            static char s_str[64] = {};
            sprintf_s(s_str, "Failure with HRESULT of %08X", result);
            return s_str;
        }

    private:
        HRESULT result;
    };

    // Helper utility converts D3D API failures into exceptions.
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw com_exception(hr);
        }
    }

    inline void Init()
    {
        ThrowIfFailed(CoInitializeEx(nullptr, 0));
    }
}