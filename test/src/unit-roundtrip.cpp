// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------

#include <catch2/catch.hpp>
#include <fx/gltf.h>
#include <nlohmann/json.hpp>
#include <string>

#include "utility.h"

void RoundtripCompare(std::string const & filePath)
{
    fx::gltf::Document doc = fx::gltf::LoadFromText(filePath);

    nlohmann::json current = doc;
    nlohmann::json original{ utility::LoadFromFile(filePath) };

    nlohmann::json diff = nlohmann::json::diff(current, original);
    nlohmann::json filteredDiff = utility::FilterDefaultElements(diff);

    if (!filteredDiff.empty())
    {
        throw std::runtime_error(filteredDiff.dump(2).c_str());
    }
}

TEST_CASE("roundtrip")
{
    SECTION("roundtrip")
    {
        std::string filePath = "data\\glTF-Sample-Models\\2.0\\Box\\glTF\\Box.gltf";

        RoundtripCompare(filePath);
    }
}
