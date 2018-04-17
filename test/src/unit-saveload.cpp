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

        REQUIRE(originalDocument.buffers.front().data == newDocument.buffers.front().data);
        REQUIRE(originalUri != newDocument.buffers.front().uri);
    }

    SECTION("load text external - save binary")
    {
        std::string originalFile{ "data/glTF-Sample-Models/2.0/Box/glTF/Box.gltf" };
        std::string newFile{ utility::GetTestOutputDir() + "/test2.glb" };

        fx::gltf::Document originalDocument = fx::gltf::LoadFromText(originalFile);

        originalDocument.buffers.front().uri.clear();
        originalDocument.Save(newFile, true);

        fx::gltf::Document newDocument = fx::gltf::LoadFromBinary(newFile);

        REQUIRE(originalDocument.buffers.front().data == newDocument.buffers.front().data);
        REQUIRE(newDocument.buffers.front().uri.empty());
    }

    SECTION("load text embedded - save external")
    {
        std::string originalFile{ "data/glTF-Sample-Models/2.0/Box/glTF-Embedded/Box.gltf" };
        std::string newFile{ utility::GetTestOutputDir() + "/test3.gltf" };

        fx::gltf::Document originalDocument = fx::gltf::LoadFromText(originalFile);
        std::string originalUri = originalDocument.buffers.front().uri;

        originalDocument.buffers.front().uri = "external.bin";
        originalDocument.Save(newFile, false);

        fx::gltf::Document newDocument = fx::gltf::LoadFromText(newFile);

        REQUIRE(originalDocument.buffers.front().data == newDocument.buffers.front().data);
        REQUIRE(originalUri != newDocument.buffers.front().uri);
        REQUIRE(newDocument.buffers.front().uri == "external.bin");
    }

    utility::CleanupTestOutputDir();
}