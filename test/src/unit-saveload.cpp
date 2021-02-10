// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------

#include <catch2/catch.hpp>
#include <fx/gltf.h>
#include <nlohmann/json.hpp>
#include <sstream>
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
        fx::gltf::Save(originalDocument, newFile, false);

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
        fx::gltf::Save(originalDocument, newFile, true);

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
        fx::gltf::Save(originalDocument, newFile, false);

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
        fx::gltf::Save(originalDocument, newFile, true);

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
        fx::gltf::Save(originalDocument, newFile, false);

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
        fx::gltf::Save(originalDocument, newFile, false);

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

        fx::gltf::Save(originalDocument, newFile, true);

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

        fx::gltf::Save(originalDocument, newFile, true);

        fx::gltf::Document newDocument = fx::gltf::LoadFromBinary(newFile);

        REQUIRE(newDocument.buffers.front().data == originalDocument.buffers.front().data);
        REQUIRE(newDocument.buffers.back().data == newBytes);
        REQUIRE(newDocument.buffers.back().IsEmbeddedResource());
    }

    SECTION("load text - save text streams")
    {
        std::string originalFile1{ "data/glTF-Sample-Models/2.0/Box/glTF/Box.gltf" };
        std::string originalFile2{ "data/glTF-Sample-Models/2.0/BoxVertexColors/glTF/BoxVertexColors.gltf" };

        fx::gltf::Document originalDocument1 = fx::gltf::LoadFromText(originalFile1);
        fx::gltf::Document originalDocument2 = fx::gltf::LoadFromText(originalFile2);

        // Roundtrip 2 different files in the same stream...
        std::stringstream ss{}; 
        fx::gltf::Save(originalDocument1, ss, utility::GetTestOutputDir(), false);
        fx::gltf::Save(originalDocument2, ss, utility::GetTestOutputDir(), false);

        ss.seekg(0, std::stringstream::beg);

        fx::gltf::Document copy1 = fx::gltf::LoadFromText(ss, utility::GetTestOutputDir());
        fx::gltf::Document copy2 = fx::gltf::LoadFromText(ss, utility::GetTestOutputDir());

        REQUIRE(copy1.buffers.front().data == originalDocument1.buffers.front().data);
        REQUIRE(copy2.buffers.front().data == originalDocument2.buffers.front().data);
    }

    SECTION("load binary - save binary streams")
    {
        std::string originalFile1{ "data/glTF-Sample-Models/2.0/Box/glTF-Binary/Box.glb" };
        std::string originalFile2{ "data/glTF-Sample-Models/2.0/BoxVertexColors/glTF-Binary/BoxVertexColors.glb" };

        fx::gltf::Document originalDocument1 = fx::gltf::LoadFromBinary(originalFile1);
        fx::gltf::Document originalDocument2 = fx::gltf::LoadFromBinary(originalFile2);

        // Roundtrip 2 different files in the same stream...
        std::stringstream ss{};
        fx::gltf::Save(originalDocument1, ss, "", true);
        fx::gltf::Save(originalDocument2, ss, "", true);

        ss.seekg(0, std::stringstream::beg);

        fx::gltf::Document copy1 = fx::gltf::LoadFromBinary(ss, "");
        fx::gltf::Document copy2 = fx::gltf::LoadFromBinary(ss, "");

        REQUIRE(copy1.buffers.front().data == originalDocument1.buffers.front().data);
        REQUIRE(copy2.buffers.front().data == originalDocument2.buffers.front().data);
    }

    utility::CleanupTestOutputDir();
}