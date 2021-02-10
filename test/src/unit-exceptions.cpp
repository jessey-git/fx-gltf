// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------

#include <catch2/catch.hpp>
#include <fx/gltf.h>
#include <nlohmann/json.hpp>
#include <sstream>

#include "utility.h"

// The matcher class
class ExceptionContainsMatcher : public Catch::MatcherBase<std::exception>
{
public:
    explicit ExceptionContainsMatcher(std::string const & text, bool shouldBeNested = false) : text_(text), shouldBeNested_(shouldBeNested) {}

    bool match(std::exception const & e) const override
    {
        fx::FormatException(message_, e);

        bool isNestedException = message_.find("nested") != std::string::npos;
        bool properlyNested = shouldBeNested_ ? isNestedException : !isNestedException;
        return properlyNested && message_.find(text_) != std::string::npos;
    }

    std::string describe() const override
    {
        std::ostringstream str;
        str << "Message '" << message_ << "' contains text '" << text_ << "' (should be nested: " << shouldBeNested_ << ")";

        return str.str();
    }

private:
    std::string text_;
    bool shouldBeNested_;

    mutable std::string message_;
};

template <typename TMutator>
void Mutate(nlohmann::json const & incoming, TMutator && mutator, std::string const & text, bool shouldBeNested = false)
{
    nlohmann::json mutated = incoming;
    mutator(mutated);

    INFO("Expecting exception containing : " << text);
    INFO("Mutated json : " << mutated.dump(2));

    REQUIRE_THROWS_MATCHES(fx::gltf::detail::Create(mutated, {}), fx::gltf::invalid_gltf_document, ExceptionContainsMatcher(text, shouldBeNested));
}

TEST_CASE("exceptions")
{
    utility::CleanupTestOutputDir();
    utility::CreateTestOutputDir();

    nlohmann::json json =
        R"(
            {
              "accessors": [ {
                  "componentType":5126, "count":14, "type":"VEC3",
                  "sparse": { "count":3, "indices": { "bufferView": 2, "componentType": 5123 }, "values":{ "bufferView": 3 } }
              } ],
              "animations": [ {
                  "channels": [ { "sampler": 0, "target": { "path": "rotation" } } ],
                  "samplers": [ { "input": 6, "interpolation": "LINEAR", "output": 7 } ]
              } ],
              "asset": { "version": "2.0" },
              "buffers": [ { "byteLength": 10, "uri": "data:application/octet-stream;base64,AAAABB==" } ],
              "bufferViews": [ { "buffer": 0, "byteLength": 10 } ],
              "cameras": [ { "perspective": { "yfov": 0.6, "znear": 1.0 }, "type": "perspective" } ],
              "images" : [ { "uri" : "data:image/jpeg;base64,$$$$" } ],
              "materials": [ { "alphaMode": "OPAQUE", "pbrMetallicRoughness": { "baseColorTexture": { "index": 0 }, "metallicRoughnessTexture": { "index": 1 } }, "normalTexture": { "index": 2 }, "occlusionTexture": { "index": 1 }, "emissiveTexture": { "index": 3 } } ],
              "meshes": [ { "primitives": [ { "attributes": { "NORMAL": 1, "POSITION": 2 } } ] } ],
              "skins": [ { "joints": [ 2 ]  } ]
            }
        )"_json;

    // Sanity check to ensure above json actually starts in a valid, loadable, state
    fx::gltf::Document mainDocument = json;
    REQUIRE(mainDocument.asset.version == "2.0");

    SECTION("load : missing required fields")
    {
        Mutate(json, [](nlohmann::json & m) { m["accessors"][0].erase("componentType"); }, "componentType");
        Mutate(json, [](nlohmann::json & m) { m["accessors"][0].erase("count"); }, "count");
        Mutate(json, [](nlohmann::json & m) { m["accessors"][0].erase("type"); }, "type");
        Mutate(json, [](nlohmann::json & m) { m["accessors"][0]["sparse"].erase("count"); }, "count");
        Mutate(json, [](nlohmann::json & m) { m["accessors"][0]["sparse"].erase("indices"); }, "indices");
        Mutate(json, [](nlohmann::json & m) { m["accessors"][0]["sparse"]["indices"].erase("bufferView"); }, "bufferView");
        Mutate(json, [](nlohmann::json & m) { m["accessors"][0]["sparse"]["indices"].erase("componentType"); }, "componentType");
        Mutate(json, [](nlohmann::json & m) { m["accessors"][0]["sparse"].erase("values"); }, "values");
        Mutate(json, [](nlohmann::json & m) { m["animations"][0].erase("channels"); }, "channels");
        Mutate(json, [](nlohmann::json & m) { m["animations"][0].erase("samplers"); }, "samplers");
        Mutate(json, [](nlohmann::json & m) { m["animations"][0]["channels"][0].erase("sampler"); }, "sampler");
        Mutate(json, [](nlohmann::json & m) { m["animations"][0]["channels"][0].erase("target"); }, "target");
        Mutate(json, [](nlohmann::json & m) { m["animations"][0]["channels"][0]["target"].erase("path"); }, "path");
        Mutate(json, [](nlohmann::json & m) { m["animations"][0]["samplers"][0].erase("input"); }, "input");
        Mutate(json, [](nlohmann::json & m) { m["animations"][0]["samplers"][0].erase("output"); }, "output");
        Mutate(json, [](nlohmann::json & m) { m["asset"].erase("version"); }, "version");
        Mutate(json, [](nlohmann::json & m) { m["buffers"][0].erase("byteLength"); }, "byteLength");
        Mutate(json, [](nlohmann::json & m) { m["bufferViews"][0].erase("buffer"); }, "buffer");
        Mutate(json, [](nlohmann::json & m) { m["bufferViews"][0].erase("byteLength"); }, "byteLength");
        Mutate(json, [](nlohmann::json & m) { m["cameras"][0].erase("type"); }, "type");
        Mutate(json, [](nlohmann::json & m) { m["cameras"][0]["perspective"].erase("yfov"); }, "yfov");
        Mutate(json, [](nlohmann::json & m) { m["cameras"][0]["perspective"].erase("znear"); }, "znear");
        Mutate(json, [](nlohmann::json & m) { m["materials"][0]["emissiveTexture"].erase("index"); }, "index");
        Mutate(json, [](nlohmann::json & m) { m["materials"][0]["pbrMetallicRoughness"]["baseColorTexture"].erase("index"); }, "index");
        Mutate(json, [](nlohmann::json & m) { m["materials"][0]["pbrMetallicRoughness"]["metallicRoughnessTexture"].erase("index"); }, "index");
        Mutate(json, [](nlohmann::json & m) { m["materials"][0]["normalTexture"].erase("index"); }, "index");
        Mutate(json, [](nlohmann::json & m) { m["materials"][0]["occlusionTexture"].erase("index"); }, "index");
        Mutate(json, [](nlohmann::json & m) { m["meshes"][0].erase("primitives"); }, "primitives");
        Mutate(json, [](nlohmann::json & m) { m["meshes"][0]["primitives"][0].erase("attributes"); }, "attributes");
        Mutate(json, [](nlohmann::json & m) { m["skins"][0].erase("joints"); }, "joints");
    }

    SECTION("load : invalid field values")
    {
        Mutate(json, [](nlohmann::json & m) { m["accessors"][0]["type"] = "vec3"; }, "accessor.type");
        Mutate(json, [](nlohmann::json & m) { m["animations"][0]["samplers"][0]["interpolation"] = "linear"; }, "animation.sampler.interpolation");
        Mutate(json, [](nlohmann::json & m) { m["buffers"][0]["byteLength"] = 0; }, "buffer.byteLength");
        Mutate(json, [](nlohmann::json & m) { m["buffers"][0]["byteLength"] = 2; }, "malformed base64");
        Mutate(json, [](nlohmann::json & m) { m["buffers"][0]["uri"] = "data:application/octet-stream;base64,$$$$"; }, "malformed base64");
        Mutate(json, [](nlohmann::json & m) { m["buffers"][0]["uri"] = "../dir/traversal.bin"; }, "buffer.uri");
        Mutate(json, [](nlohmann::json & m) { m["buffers"][0]["uri"] = "nonexistant.bin"; }, "buffer.uri");
        Mutate(json, [](nlohmann::json & m) { m["cameras"][0]["type"] = "D-SLR"; }, "camera.type");
        Mutate(json, [](nlohmann::json & m) { m["materials"][0]["alphaMode"] = "opaque"; }, "material.alphaMode");

        std::vector<uint8_t> data{};
        REQUIRE_THROWS_MATCHES(mainDocument.images[0].MaterializeData(data), fx::gltf::invalid_gltf_document, ExceptionContainsMatcher("malformed base64"));
    }

    SECTION("load : quotas")
    {
        std::string externalFile{ "data/glTF-Sample-Models/2.0/Box/glTF/Box.gltf" };
        std::string glbFile{ "data/glTF-Sample-Models/2.0/Box/glTF-Binary/Box.glb" };

        fx::gltf::ReadQuotas readQuotas{};
        readQuotas.MaxBufferByteLength = 500;
        REQUIRE_THROWS_MATCHES(fx::gltf::LoadFromText(externalFile, readQuotas), fx::gltf::invalid_gltf_document, ExceptionContainsMatcher("MaxBufferByteLength"));

        readQuotas.MaxFileSize = 500;
        REQUIRE_THROWS_MATCHES(fx::gltf::LoadFromBinary(glbFile, readQuotas), fx::gltf::invalid_gltf_document, ExceptionContainsMatcher("MaxFileSize"));

        readQuotas = {};
        readQuotas.MaxBufferCount = 1;
        nlohmann::json external = utility::LoadJsonFromFile(externalFile);
        external["buffers"].push_back(json["buffers"][0]); // Duplicate so we have 2 identical buffers
        REQUIRE_THROWS_MATCHES(fx::gltf::detail::Create(external, { fx::gltf::detail::GetDocumentRootPath(externalFile), readQuotas }), fx::gltf::invalid_gltf_document, ExceptionContainsMatcher("MaxBufferCount"));
    }

    SECTION("load : mismatched")
    {
        REQUIRE_THROWS_MATCHES(fx::gltf::LoadFromText("data/glTF-Sample-Models/2.0/Box/glTF-Binary/Box.glb"), fx::gltf::invalid_gltf_document, ExceptionContainsMatcher("json.exception", true));
        REQUIRE_THROWS_MATCHES(fx::gltf::LoadFromBinary("data/glTF-Sample-Models/2.0/Box/glTF/Box.gltf"), fx::gltf::invalid_gltf_document, ExceptionContainsMatcher("GLB header"));
    }

    SECTION("load / save : invalid path")
    {
        fx::gltf::Document doc;
        doc = json;
        doc.buffers[0].data.resize(doc.buffers[0].byteLength);
        doc.buffers.push_back(doc.buffers[0]);
        doc.buffers[0].uri.clear();

        REQUIRE_THROWS_AS(fx::gltf::LoadFromText("not-exist"), std::system_error);
        REQUIRE_THROWS_AS(fx::gltf::LoadFromBinary("not-exist"), std::system_error);

        REQUIRE_THROWS_AS(fx::gltf::Save(doc, utility::GetTestOutputDir() + "/./", false), std::system_error);
        REQUIRE_THROWS_AS(fx::gltf::Save(doc, utility::GetTestOutputDir() + "/./", true), std::system_error);

        doc.buffers[1].uri = "./";
        REQUIRE_THROWS_AS(fx::gltf::Save(doc, utility::GetTestOutputDir() + "/nop", true), fx::gltf::invalid_gltf_document);
    }

    SECTION("save : invalid buffers")
    {
        fx::gltf::Document doc;

        INFO("No buffers");
        doc = json;
        doc.buffers.clear();
        REQUIRE_THROWS_AS(fx::gltf::Save(doc, utility::GetTestOutputDir() + "/nop", false), fx::gltf::invalid_gltf_document);

        INFO("Buffer byteLength = 0");
        doc = json;
        doc.buffers[0].byteLength = 0;
        REQUIRE_THROWS_AS(fx::gltf::Save(doc, utility::GetTestOutputDir() + "/nop", false), fx::gltf::invalid_gltf_document);

        INFO("Buffer byteLength != data size");
        doc = json;
        doc.buffers[0].byteLength = 20;
        doc.buffers[0].data.resize(10);
        REQUIRE_THROWS_AS(fx::gltf::Save(doc, utility::GetTestOutputDir() + "/nop", false), fx::gltf::invalid_gltf_document);

        INFO("A second buffer with empty uri");
        doc = json;
        doc.buffers.push_back({});
        doc.buffers[0].uri.clear();
        doc.buffers[0].byteLength = 20;
        doc.buffers[0].data.resize(20);
        doc.buffers[1].uri.clear();
        doc.buffers[1].byteLength = 20;
        doc.buffers[1].data.resize(20);
        REQUIRE_THROWS_AS(fx::gltf::Save(doc, utility::GetTestOutputDir() + "/nop", false), fx::gltf::invalid_gltf_document);

        INFO("Binary save with invalid buffer uri");
        doc = json;
        doc.buffers[0].uri = "not empty";
        doc.buffers[0].byteLength = 20;
        doc.buffers[0].data.resize(20);
        REQUIRE_THROWS_AS(fx::gltf::Save(doc, utility::GetTestOutputDir() + "/nop", true), fx::gltf::invalid_gltf_document);
    }

    utility::CleanupTestOutputDir();
}