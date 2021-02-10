// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <exception>
#include <nlohmann/json.hpp>
#include <string>

namespace utility
{
    std::string GetTestOutputDir();
    void CreateTestOutputDir();
    void CleanupTestOutputDir();

    nlohmann::json LoadJsonFromFile(std::string const & filePath);

    // Many .glTF files contain json elements which are optional.
    // When performing roundtrip tests against such files, fx-gltf will
    // remove these optional elements in the final json, causing the diff
    // to fail. This function performs the diff and filters such cases.
    nlohmann::json DiffAndFilter(nlohmann::json const & current, nlohmann::json const & original);

} // namespace utility