// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------

#include "utility.h"

#include <fstream>
#include <system_error>

namespace utility
{
    void FormatException(std::string & output, std::exception const & e, int level)
    {
        output.append(std::string(level, ' ')).append("exception: ").append(e.what()).append("\n");
        try
        {
            std::rethrow_if_nested(e);
        }
        catch (const std::exception& e)
        {
            FormatException(output, e, level + 1);
        }
        catch (...) {}
    }

    nlohmann::json LoadJsonFromFile(std::string const & filePath)
    {
        nlohmann::json result{};
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            throw std::system_error(std::make_error_code(std::errc::no_such_file_or_directory));
        }

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
                    element["path"].get<std::string>().find("translation") != std::string::npos && element["value"].get<std::vector<float>>() == std::vector<float>{ 0.0f, 0.0f, 0.0f } ||
                    element["path"].get<std::string>().find("rotation") != std::string::npos && element["value"].get<std::vector<float>>() == std::vector<float>{ 0.0f, 0.0f, 0.0f, 1.0f } ||
                    element["path"].get<std::string>().find("scale") != std::string::npos && element["value"].get<std::vector<float>>() == std::vector<float>{ 1.0f, 1.0f, 1.0f } ||
                    element["path"].get<std::string>().find("mode") != std::string::npos && element["value"] == 4 ||
                    element["path"].get<std::string>().find("byteOffset") != std::string::npos && element["value"] == 0 ||
                    element["path"].get<std::string>().find("interpolation") != std::string::npos && element["value"] == "LINEAR" ||
                    element["path"].get<std::string>().find("wrapS") != std::string::npos && element["value"] == 10497 ||
                    element["path"].get<std::string>().find("wrapT") != std::string::npos && element["value"] == 10497 ||
                    element["path"].get<std::string>().find("doubleSided") != std::string::npos && element["value"] == false)
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

                if (element["value"].size() == 0)
                {
                    continue;
                }
            }

            filtered.push_back(element);
        }

        return filtered;
    }
} // namespace utility