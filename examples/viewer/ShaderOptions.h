// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once
#include <cstdint>
#include <string>
#include <vector>

enum class ShaderOptions : uint64_t
{
    None = 0x0u,
    USE_AUTO_COLOR = 0x1u,
    USE_FACTORS_ONLY = 0x2u,
    USE_MANUAL_SRGB = 0x4u,
    USE_IBL = 0x8u,
    HAS_BASECOLORMAP = 0x10u,
    HAS_NORMALMAP = 0x20u,
    HAS_METALROUGHNESSMAP = 0x40u,
    HAS_OCCLUSIONMAP = 0x80u,
    HAS_OCCLUSIONMAP_COMBINED = 0x100u,
    HAS_EMISSIVEMAP = 0x200u
};

inline ShaderOptions operator|(ShaderOptions a, ShaderOptions b)
{
    return ShaderOptions(((uint64_t)a) | ((uint64_t)b));
}

inline ShaderOptions & operator|=(ShaderOptions & a, ShaderOptions b)
{
    return (ShaderOptions &)(((uint64_t &)a) |= ((uint64_t)b));
}

inline ShaderOptions operator&(ShaderOptions a, ShaderOptions b)
{
    return ShaderOptions(((uint64_t)a) & ((uint64_t)b));
}

inline ShaderOptions & operator&=(ShaderOptions & a, ShaderOptions b)
{
    return (ShaderOptions &)(((uint64_t &)a) &= ((uint64_t)b));
}

inline ShaderOptions operator~(ShaderOptions a)
{
    return ShaderOptions(~((uint64_t)a));
}

inline ShaderOptions operator^(ShaderOptions a, ShaderOptions b)
{
    return ShaderOptions(((uint64_t)a) ^ ((uint64_t)b));
}

inline ShaderOptions & operator^=(ShaderOptions & a, ShaderOptions b)
{
    return (ShaderOptions &)(((uint64_t &)a) ^= ((uint64_t)b));
}

inline bool IsSet(ShaderOptions options, ShaderOptions flag)
{
    return (options & flag) == flag;
}

inline std::vector<std::string> GetShaderDefines(ShaderOptions options)
{
    std::vector<std::string> defines;

    if (IsSet(options, ShaderOptions::USE_AUTO_COLOR))
    {
        defines.emplace_back("USE_AUTO_COLOR");
    }
    if (IsSet(options, ShaderOptions::USE_FACTORS_ONLY))
    {
        defines.emplace_back("USE_FACTORS_ONLY");
    }
    if (IsSet(options, ShaderOptions::USE_MANUAL_SRGB))
    {
        defines.emplace_back("USE_MANUAL_SRGB");
    }
    if (IsSet(options, ShaderOptions::USE_IBL))
    {
        defines.emplace_back("USE_IBL");
    }
    if (IsSet(options, ShaderOptions::HAS_BASECOLORMAP))
    {
        defines.emplace_back("HAS_BASECOLORMAP");
    }
    if (IsSet(options, ShaderOptions::HAS_NORMALMAP))
    {
        defines.emplace_back("HAS_NORMALMAP");
    }
    if (IsSet(options, ShaderOptions::HAS_METALROUGHNESSMAP))
    {
        defines.emplace_back("HAS_METALROUGHNESSMAP");
    }
    if (IsSet(options, ShaderOptions::HAS_OCCLUSIONMAP))
    {
        defines.emplace_back("HAS_OCCLUSIONMAP");
    }
    if (IsSet(options, ShaderOptions::HAS_OCCLUSIONMAP_COMBINED))
    {
        if (!IsSet(options, ShaderOptions::HAS_METALROUGHNESSMAP))
        {
            throw std::runtime_error("Invalid ShaderOptions. HAS_OCCLUSIONMAP_COMBINED requires HAS_METALROUGHNESSMAP");
        }

        defines.emplace_back("HAS_OCCLUSIONMAP_COMBINED");
    }
    if (IsSet(options, ShaderOptions::HAS_EMISSIVEMAP))
    {
        defines.emplace_back("HAS_EMISSIVEMAP");
    }

    return defines;
}