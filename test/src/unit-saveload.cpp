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

TEST_CASE("saveload")
{
    utility::CleanupTestOutputDir();
    utility::CreateTestOutputDir();

    SECTION("load text external - save text embedded")
    {
        std::string originalFile{ "data/glTF-Sample-Models/2.0/Box/glTF/Box.gltf" };
        std::string newFile{ utility::GetTestOutputDir() + "/test1.gltf" };

        fx::gltf::Document originalDocument = fx::gltf::LoadFromText(originalFile);
        std::string originalUri = originalDocument.buffers.front().uri;

        originalDocument.buffers.front().SetEmbeddedResource();
        originalDocument.Save(newFile, false);

        fx::gltf::Document newDocument = fx::gltf::LoadFromText(newFile);

        REQUIRE(newDocument.buffers.front().data == originalDocument.buffers.front().data);
        REQUIRE(newDocument.buffers.front().uri != originalUri);
        REQUIRE(newDocument.buffers.front().IsEmbeddedResource());
    }

    SECTION("load text external - save binary")
    {
        std::string originalFile{ "data/glTF-Sample-Models/2.0/Box/glTF/Box.gltf" };
        std::string newFile{ utility::GetTestOutputDir() + "/test2.glb" };

        fx::gltf::Document originalDocument = fx::gltf::LoadFromText(originalFile);

        originalDocument.buffers.front().uri.clear();
        originalDocument.Save(newFile, true);

        fx::gltf::Document newDocument = fx::gltf::LoadFromBinary(newFile);

        REQUIRE(newDocument.buffers.front().data == originalDocument.buffers.front().data);
        REQUIRE(newDocument.buffers.front().uri.empty());
    }

    SECTION("load text embedded - save text external")
    {
        std::string originalFile{ "data/glTF-Sample-Models/2.0/Box/glTF-Embedded/Box.gltf" };
        std::string newFile{ utility::GetTestOutputDir() + "/test3.gltf" };

        fx::gltf::Document originalDocument = fx::gltf::LoadFromText(originalFile);
        std::string originalUri = originalDocument.buffers.front().uri;

        originalDocument.buffers.front().uri = "test3.bin";
        originalDocument.Save(newFile, false);

        fx::gltf::Document newDocument = fx::gltf::LoadFromText(newFile);

        REQUIRE(newDocument.buffers.front().data == originalDocument.buffers.front().data);
        REQUIRE(newDocument.buffers.front().uri != originalUri);
        REQUIRE(newDocument.buffers.front().uri == "test3.bin");
    }

    SECTION("load text embedded - save binary")
    {
        std::string originalFile{ "data/glTF-Sample-Models/2.0/Box/glTF-Embedded/Box.gltf" };
        std::string newFile{ utility::GetTestOutputDir() + "/test4.glb" };

        fx::gltf::Document originalDocument = fx::gltf::LoadFromText(originalFile);

        originalDocument.buffers.front().uri.clear();
        originalDocument.Save(newFile, true);

        fx::gltf::Document newDocument = fx::gltf::LoadFromBinary(newFile);

        REQUIRE(newDocument.buffers.front().data == originalDocument.buffers.front().data);
        REQUIRE(newDocument.buffers.front().uri.empty());
    }

    SECTION("load binary - save text external")
    {
        std::string originalFile{ "data/glTF-Sample-Models/2.0/Box/glTF-Binary/Box.glb" };
        std::string newFile{ utility::GetTestOutputDir() + "/test5.gltf" };

        fx::gltf::Document originalDocument = fx::gltf::LoadFromBinary(originalFile);

        originalDocument.buffers.front().uri = "test5.bin";
        originalDocument.Save(newFile, false);

        fx::gltf::Document newDocument = fx::gltf::LoadFromText(newFile);

        REQUIRE(newDocument.buffers.front().data == originalDocument.buffers.front().data);
        REQUIRE(newDocument.buffers.front().uri == "test5.bin");
    }

    SECTION("load binary - save text embedded")
    {
        std::string originalFile{ "data/glTF-Sample-Models/2.0/Box/glTF-Binary/Box.glb" };
        std::string newFile{ utility::GetTestOutputDir() + "/test6.gltf" };

        fx::gltf::Document originalDocument = fx::gltf::LoadFromBinary(originalFile);

        originalDocument.buffers.front().SetEmbeddedResource();
        originalDocument.Save(newFile, false);

        fx::gltf::Document newDocument = fx::gltf::LoadFromText(newFile);

        REQUIRE(newDocument.buffers.front().data == originalDocument.buffers.front().data);
        REQUIRE(newDocument.buffers.front().IsEmbeddedResource());
    }

    SECTION("load binary - save binary + additional as external")
    {
        std::string originalFile{ "data/glTF-Sample-Models/2.0/Box/glTF-Binary/Box.glb" };
        std::string newFile{ utility::GetTestOutputDir() + "/test7.glb" };

        fx::gltf::Document originalDocument = fx::gltf::LoadFromBinary(originalFile);

        std::vector<uint8_t> newBytes = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        originalDocument.buffers.push_back(fx::gltf::Buffer{});
        originalDocument.buffers.back().uri = "test7.bin";
        originalDocument.buffers.back().byteLength = static_cast<uint32_t>(newBytes.size());
        originalDocument.buffers.back().data = newBytes;

        originalDocument.Save(newFile, true);

        fx::gltf::Document newDocument = fx::gltf::LoadFromBinary(newFile);

        REQUIRE(newDocument.buffers.front().data == originalDocument.buffers.front().data);
        REQUIRE(newDocument.buffers.back().data == newBytes);
    }

    SECTION("load binary - save binary + additional as embedded")
    {
        std::string originalFile{ "data/glTF-Sample-Models/2.0/Box/glTF-Binary/Box.glb" };
        std::string newFile{ utility::GetTestOutputDir() + "/test8.glb" };

        fx::gltf::Document originalDocument = fx::gltf::LoadFromBinary(originalFile);

        std::vector<uint8_t> newBytes = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        originalDocument.buffers.push_back(fx::gltf::Buffer{});
        originalDocument.buffers.back().uri = "test8.bin";
        originalDocument.buffers.back().byteLength = static_cast<uint32_t>(newBytes.size());
        originalDocument.buffers.back().data = newBytes;
        originalDocument.buffers.back().SetEmbeddedResource();

        originalDocument.Save(newFile, true);

        fx::gltf::Document newDocument = fx::gltf::LoadFromBinary(newFile);

        REQUIRE(newDocument.buffers.front().data == originalDocument.buffers.front().data);
        REQUIRE(newDocument.buffers.back().data == newBytes);
        REQUIRE(newDocument.buffers.back().IsEmbeddedResource());
    }

    utility::CleanupTestOutputDir();
}