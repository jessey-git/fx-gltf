// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <exception>
#include <nlohmann/json.hpp>

#ifndef FX_GLTF_FILESYSTEM
    #if defined(__clang__)
        #if __clang_major__ < 7 || (defined(__cplusplus) && __cplusplus < 201703L)
            #define FX_GLTF_EXPERIMENTAL_FILESYSTEM
        #endif
    #elif defined(__GNUC__)
        #if __GNUC__ < 8 || (defined(__cplusplus) && __cplusplus < 201703L)
            #define FX_GLTF_EXPERIMENTAL_FILESYSTEM
        #endif
    #elif defined(_MSC_VER)
        #if _MSC_VER < 1914 || (!defined(_HAS_CXX17) || (defined(_HAS_CXX17) && _HAS_CXX17 == 0))
            #define FX_GLTF_EXPERIMENTAL_FILESYSTEM
        #endif
    #endif

    #ifdef FX_GLTF_EXPERIMENTAL_FILESYSTEM
        #include <experimental/filesystem>
        #define FX_GLTF_FILESYSTEM std::experimental::filesystem::v1
    #else
        #include <filesystem>
        #define FX_GLTF_FILESYSTEM std::filesystem
    #endif
#endif

namespace utility
{
    FX_GLTF_FILESYSTEM::path GetTestOutputDir();
    void CreateTestOutputDir();
    void CleanupTestOutputDir();

    nlohmann::json LoadJsonFromFile(FX_GLTF_FILESYSTEM::path const & filePath);

    // Many .glTF files contain json elements which are optional.
    // When performing roundtrip tests against such files, fx-gltf will
    // remove these optional elements in the final json, causing the diff
    // to fail. This function performs the diff and filters such cases.
    nlohmann::json DiffAndFilter(nlohmann::json const & current, nlohmann::json const & original);

} // namespace utility