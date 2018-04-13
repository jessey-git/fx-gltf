// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------

#include "utility.h"

#include <fstream>

namespace utility
{
    nlohmann::json LoadFromFile(std::string const & filePath)
    {
        nlohmann::json result{};
        std::ifstream file(filePath);
        file >> result;

        return result;
    }

    nlohmann::json FilterDefaultElements(nlohmann::json const & incoming)
    {
        nlohmann::json filtered{};
        for (auto & element : incoming)
        {
            if (element["op"] == "add")
            {
                if (element["path"].get<std::string>().find("emissiveFactor") != std::string::npos && element["value"].get<std::vector<float>>() == std::vector<float>{ 0.0f, 0.0f, 0.0f } ||
                    element["path"].get<std::string>().find("baseColorFactor") != std::string::npos && element["value"].get<std::vector<float>>() == std::vector<float>{ 1.0f, 1.0f, 1.0f, 1.0f } ||
                    element["path"].get<std::string>().find("mode") != std::string::npos && element["value"] == 4 ||
                    element["path"].get<std::string>().find("byteOffset") != std::string::npos && element["value"] == 0)
                {
                    continue;
                }
            }
            else if (element["op"] == "replace")
            {
                if (element["value"].is_number_float())
                {
                    continue;
                }
            }

            filtered.push_back(element);
        }

        return filtered;
    }
} // namespace utility