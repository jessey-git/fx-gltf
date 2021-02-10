// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------

#include <catch2/catch.hpp>
#include <fx/gltf.h>
#include <nlohmann/json.hpp>
#include <random>
#include <string>
#include <vector>

TEST_CASE("base64")
{
    SECTION("simple")
    {
        std::vector<uint8_t> bytes{};
        std::string base64Text{};

        base64Text = "";
        REQUIRE(fx::base64::TryDecode(base64Text, bytes));
        REQUIRE(fx::base64::Encode(bytes) == base64Text);
        REQUIRE(bytes.empty());

        base64Text = "TEST";
        REQUIRE(fx::base64::TryDecode(base64Text, bytes));
        REQUIRE(fx::base64::Encode(bytes) == base64Text);
        REQUIRE(bytes == std::vector<uint8_t>{ 76, 68, 147 });

        base64Text = "YW55IGNhcm5hbCBwbGVhcw==";
        REQUIRE(fx::base64::TryDecode(base64Text, bytes));
        REQUIRE(fx::base64::Encode(bytes) == base64Text);
        REQUIRE(bytes == std::vector<uint8_t>{ 97, 110, 121, 32, 99, 97, 114, 110, 97, 108, 32, 112, 108, 101, 97, 115 });
    }

    SECTION("random encode/decode")
    {
        std::mt19937 gen(29);
        std::uniform_int_distribution<> dist(0, 255);

        for (std::size_t iteration = 1; iteration < 1024; iteration++)
        {
            // Random bytes...
            std::vector<uint8_t> bytes(iteration);
            std::generate(bytes.begin(), bytes.end(), [&dist, &gen] { return static_cast<char>(dist(gen)); });

            // Roundtrip...
            std::string encoded = fx::base64::Encode(bytes);

            std::vector<uint8_t> outBytes;
            const bool decodeResult = fx::base64::TryDecode(encoded, outBytes);

            REQUIRE(decodeResult);
            REQUIRE(bytes == outBytes);
            REQUIRE(outBytes.size() == iteration);
        }
    }

    SECTION("random negative")
    {
        std::mt19937 gen(29);
        std::uniform_int_distribution<> dist(0, 255);

        std::vector<uint8_t> invalidChars(192);
        for (auto c : fx::base64::detail::DecodeMap)
        {
            if (c == -1)
            {
                invalidChars.push_back(c);
            }
        }

        for (std::size_t iteration = 1; iteration < 1024; iteration++)
        {
            // Random, valid, encoded string...
            std::string encoded(iteration, '\0');
            std::generate(encoded.begin(), encoded.end(), [&dist, &gen] { return fx::base64::detail::EncodeMap[dist(gen) % 64]; });

            bool decodeResult = false;
            std::vector<uint8_t> outBytes;
            if (encoded.length() % 4 == 0)
            {
                // Base string should decode fine...
                decodeResult = fx::base64::TryDecode(encoded, outBytes);
                REQUIRE(decodeResult);

                // Mutate it to force some form of failure...
                for (int mutation = 0; mutation < 64; mutation++)
                {
                    std::string invalid = encoded;
                    std::size_t mutatePosition = dist(gen) % invalid.length();
                    invalid[mutatePosition] = invalidChars[dist(gen) % invalidChars.size()];

                    decodeResult = fx::base64::TryDecode(invalid, outBytes);
                    REQUIRE_FALSE(decodeResult);
                }
            }
            else
            {
                // Strings whose length are not divisible by 4 should always fail
                decodeResult = fx::base64::TryDecode(encoded, outBytes);
                REQUIRE_FALSE(decodeResult);
            }
        }
    }

    SECTION("negative")
    {
        std::vector<uint8_t> bytes{};
        std::string base64Text{};

        base64Text = "A";
        REQUIRE_FALSE(fx::base64::TryDecode(base64Text, bytes));
        REQUIRE(bytes.empty());

        base64Text = "AA";
        REQUIRE_FALSE(fx::base64::TryDecode(base64Text, bytes));
        REQUIRE(bytes.empty());

        base64Text = "AAA";
        REQUIRE_FALSE(fx::base64::TryDecode(base64Text, bytes));
        REQUIRE(bytes.empty());

        base64Text = "AA A";
        REQUIRE_FALSE(fx::base64::TryDecode(base64Text, bytes));
        REQUIRE(bytes.empty());

        base64Text = "====";
        REQUIRE_FALSE(fx::base64::TryDecode(base64Text, bytes));
        REQUIRE(bytes.empty());

        base64Text = "A===";
        REQUIRE_FALSE(fx::base64::TryDecode(base64Text, bytes));
        REQUIRE(bytes.empty());

        base64Text = "A==A";
        REQUIRE_FALSE(fx::base64::TryDecode(base64Text, bytes));
        REQUIRE(bytes.empty());

        base64Text = "AA=A";
        REQUIRE_FALSE(fx::base64::TryDecode(base64Text, bytes));
        REQUIRE(bytes.empty());

        base64Text = "AAA[";
        REQUIRE_FALSE(fx::base64::TryDecode(base64Text, bytes));
        REQUIRE(bytes.empty());

        base64Text = "YW55IGNhcm5hbCBwbGVhcw=";
        REQUIRE_FALSE(fx::base64::TryDecode(base64Text, bytes));
        REQUIRE(bytes.empty());

        base64Text = "YW55IGNhcm5hbCBwbGVhcw";
        REQUIRE_FALSE(fx::base64::TryDecode(base64Text, bytes));
        REQUIRE(bytes.empty());
    }
}