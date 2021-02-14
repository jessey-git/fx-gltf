// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------

#include "utility.h"

#include "catch2/catch.hpp"

#include <cfloat>
#include <fstream>
#include <system_error>

#if (defined(__clang__)) || \
    (defined(__GNUC__) && ((__GNUC__ < 8) || (defined(__cplusplus) && __cplusplus < 201703L))) || \
    (defined(_MSC_VER) && ((_MSC_VER < 1914) || (!defined(_HAS_CXX17) || (defined(_HAS_CXX17) && _HAS_CXX17 == 0))))
    #include <experimental/filesystem>
    #define FX_GLTF_FILESYSTEM std::experimental::filesystem::v1
#else
    #include <filesystem>
    #define FX_GLTF_FILESYSTEM std::filesystem
#endif

namespace utility
{
    std::string GetTestOutputDir()
    {
        return "output";
    }

    void CreateTestOutputDir()
    {
        std::error_code err{};
        int attempts = 5;
        while (--attempts >= 0)
        {
            if (FX_GLTF_FILESYSTEM::create_directory(GetTestOutputDir(), err))
            {
                break;
            }
        }
    }

    void CleanupTestOutputDir()
    {
        std::error_code err{};
        int attempts = 5;
        while (--attempts >= 0)
        {
            if (FX_GLTF_FILESYSTEM::remove_all(GetTestOutputDir(), err) != static_cast<std::uintmax_t>(-1))
            {
                break;
            }
        }
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

    // https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
    union Float_t
    {
        explicit Float_t(float num = 0.0f)
            : f(num) {}
        bool Negative() const { return i < 0; }

        int32_t i;
        float f;
    };

    bool AlmostEqual(float A, float B)
    {
        // Check if the numbers are really close -- needed
        // when comparing numbers near zero.
        float absDiff = std::fabs(A - B);
        if (absDiff <= 1e-5f)
        {
            return true;
        }

        float absA = std::fabs(A);
        float absB = std::fabs(B);
        float largest = (absB > absA) ? absB : absA;
        const float maxDiff = largest * FLT_EPSILON;

        if (absDiff <= maxDiff)
        {
            return true;
        }

        Float_t uA(A);
        Float_t uB(B);

        // Different signs means they do not match.
        if (uA.Negative() != uB.Negative())
        {
            return false;
        }

        // Find the difference in ULPs.
        int ulpsDiff = abs(uA.i - uB.i);

        return ulpsDiff <= 2;
    }

    nlohmann::json DiffAndFilter(nlohmann::json const & current, nlohmann::json const & original)
    {
        nlohmann::json filtered{};
        nlohmann::json diff = nlohmann::json::diff(current, original);

        for (auto & element : diff)
        {
            std::string path = element["path"].get<std::string>();

            if (element["op"] == "add")
            {
                if ((path.find("emissiveFactor") != std::string::npos && element["value"].get<std::vector<float>>() == std::vector<float>{ 0.0f, 0.0f, 0.0f }) ||
                    (path.find("baseColorFactor") != std::string::npos && element["value"].get<std::vector<float>>() == std::vector<float>{ 1.0f, 1.0f, 1.0f, 1.0f }) ||
                    (path.find("translation") != std::string::npos && element["value"].get<std::vector<float>>() == std::vector<float>{ 0.0f, 0.0f, 0.0f }) ||
                    (path.find("rotation") != std::string::npos && element["value"].get<std::vector<float>>() == std::vector<float>{ 0.0f, 0.0f, 0.0f, 1.0f }) ||
                    (path.find("mode") != std::string::npos && element["value"] == 4) ||
                    (path.find("byteOffset") != std::string::npos && element["value"] == 0) ||
                    (path.find("interpolation") != std::string::npos && element["value"] == "LINEAR") ||
                    (path.find("wrapS") != std::string::npos && element["value"] == 10497) ||
                    (path.find("wrapT") != std::string::npos && element["value"] == 10497) ||
                    (path.find("doubleSided") != std::string::npos && element["value"] == false) ||
                    (path.find("metallicFactor") != std::string::npos && element["value"] == 1.0f) ||
                    (path.find("roughnessFactor") != std::string::npos && element["value"] == 1.0f) ||
                    (path.find("strength") != std::string::npos && element["value"] == 1.0f) ||
                    (path.find("alphaMode") != std::string::npos && element["value"] == "OPAQUE") ||
                    (path.find("texCoord") != std::string::npos && element["value"] == 0))
                {
                    continue;
                }

                if (path.find("scale") != std::string::npos)
                {
                    if ((element["value"].is_array() && element["value"].get<std::vector<float>>() == std::vector<float>{ 1.0f, 1.0f, 1.0f }) ||
                        (element["value"].is_number() && element["value"].get<float>() == 1.0f))
                    {
                        continue;
                    }
                }
            }
            else if (element["op"] == "replace")
            {
                if (path.find("nodes") != std::string::npos)
                {
                    if (element["value"].find("rotation") != element["value"].end())
                    {
                        if (element["value"]["rotation"].get<std::vector<float>>() == std::vector<float>{ 0.0f, 0.0f, 0.0f, 1.0f })
                        {
                            continue;
                        }
                    }
                }

                if (element["value"].is_number_float())
                {
                    // The text-based json diff says our numbers are mismatched but that could be
                    // because the text actually has more digits of precision than what can fit in
                    // a float. Let's just double-check aginst current and original...
                    float currentValue = current[nlohmann::json::json_pointer(path)].get<float>();
                    float originalValue = original[nlohmann::json::json_pointer(path)].get<float>();
                    if (AlmostEqual(currentValue, originalValue))
                    {
                        continue;
                    }
                }

                if (element["value"].empty())
                {
                    continue;
                }
            }

            filtered.push_back(element);
        }

        return filtered;
    }
} // namespace utility