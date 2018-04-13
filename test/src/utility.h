// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace utility
{
    nlohmann::json LoadFromFile(std::string const & filePath);

    nlohmann::json FilterDefaultElements(nlohmann::json const & incoming);
} // namespace utility