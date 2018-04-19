// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------

#include <catch2/catch.hpp>
#include <fx/gltf.h>
#include <nlohmann/json.hpp>

// The matcher class
class ExceptionContainsMatcher : public Catch::MatcherBase<std::exception>
{
public:
    ExceptionContainsMatcher(std::string text) : text_(text) {}

    virtual bool match(std::exception const & e) const override
    {
        message_.assign(e.what());
        return message_.find(text_) != std::string::npos;
    }

    virtual std::string describe() const 
    {
        return std::string("Message '").append(message_).append("' contains text '").append(text_).append("'");
    }

private:
    std::string text_;
    mutable std::string message_;
};

template <typename TMutator>
void Mutate(nlohmann::json const & incoming, TMutator && mutator, std::string const & text)
{
    nlohmann::json mutated = incoming;
    mutator(mutated);

    INFO("Expecting exception containing : " << text);
    INFO("Mutated json : " << mutated.dump(2));

    fx::gltf::Document doc;
    REQUIRE_THROWS_MATCHES(doc = mutated, fx::gltf::invalid_gltf_document, ExceptionContainsMatcher(text));
}

TEST_CASE("exceptions")
{
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
              "buffers": [ { "byteLength": 10 } ],
              "bufferViews": [ { "buffer": 0, "byteLength": 10 } ],
              "cameras": [ { "perspective": { "yfov": 0.6, "znear": 1.0 }, "type": "perspective" } ],
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
        Mutate(json, [](nlohmann::json & m) { m["cameras"][0]["type"] = "D-SLR"; }, "camera.type");
        Mutate(json, [](nlohmann::json & m) { m["materials"][0]["alphaMode"] = "opaque"; }, "material.alphaMode");
    }

    SECTION("save : invalid buffers")
    {
        fx::gltf::Document doc;

        INFO("No buffers");
        doc = json;
        doc.buffers.clear();
        REQUIRE_THROWS_AS(doc.Save("nop", false), fx::gltf::invalid_gltf_document);

        INFO("Buffer byteLength = 0");
        doc = json;
        doc.buffers[0].byteLength = 0;
        REQUIRE_THROWS_AS(doc.Save("nop", false), fx::gltf::invalid_gltf_document);

        INFO("Buffer byteLength != data size");
        doc = json;
        doc.buffers[0].byteLength = 20;
        doc.buffers[0].data.resize(10);
        REQUIRE_THROWS_AS(doc.Save("nop", false), fx::gltf::invalid_gltf_document);

        INFO("A second buffer with empty uri");
        doc = json;
        doc.buffers.push_back({});
        doc.buffers[0].uri.clear();
        doc.buffers[0].byteLength = 20;
        doc.buffers[0].data.resize(20);
        doc.buffers[1].uri.clear();
        doc.buffers[1].byteLength = 20;
        doc.buffers[1].data.resize(20);
        REQUIRE_THROWS_AS(doc.Save("nop", false), fx::gltf::invalid_gltf_document);

        INFO("Binary save with invalid buffer uri");
        doc = json;
        doc.buffers[0].uri = "not empty";
        doc.buffers[0].byteLength = 20;
        doc.buffers[0].data.resize(20);
        REQUIRE_THROWS_AS(doc.Save("nop", true), fx::gltf::invalid_gltf_document);
    }
}