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

void RoundtripCompare(std::string const & filePath)
{
    INFO(filePath);

    std::string errorString = "Failed: ";
    errorString.append(filePath).append("\n");

    try
    {
        fx::gltf::Document doc = fx::gltf::LoadFromText(filePath);

        nlohmann::json current = doc;
        nlohmann::json original{ utility::LoadJsonFromFile(filePath) };

        nlohmann::json diff = nlohmann::json::diff(current, original);
        nlohmann::json filteredDiff = utility::FilterDefaultElements(diff);

        if (!filteredDiff.empty())
        {
            throw std::runtime_error(errorString.append(filteredDiff.dump(2)));
        }

        // Ensure all the buffers have data...
        for (auto b : doc.buffers)
        {
            REQUIRE(b.data.size() == b.byteLength);
        }
    }
    catch (fx::gltf::invalid_gltf_document & e)
    {
        utility::FormatException(errorString, e);

        throw std::runtime_error(errorString);
    }
}

void CheckBinary(std::string const & filePath)
{
    INFO(filePath);

    std::string errorString = "Failed: ";
    errorString.append(filePath).append("\n");

    try
    {
        fx::gltf::Document doc = fx::gltf::LoadFromBinary(filePath);

        // Ensure all the buffers have data...
        for (auto & b : doc.buffers)
        {
            REQUIRE(b.data.size() == b.byteLength);
        }
    }
    catch (fx::gltf::invalid_gltf_document & e)
    {
        utility::FormatException(errorString, e);

        throw std::runtime_error(errorString);
    }
}

TEST_CASE("roundtrip")
{
    SECTION("roundtrip - .gltf files w/external resources")
    {
        for (auto & filePath :
            {
                "data/glTF-Sample-Models/2.0/2CylinderEngine/glTF/2CylinderEngine.gltf",
                "data/glTF-Sample-Models/2.0/AlphaBlendModeTest/glTF/AlphaBlendModeTest.gltf",
                "data/glTF-Sample-Models/2.0/AnimatedCube/glTF/AnimatedCube.gltf",
                "data/glTF-Sample-Models/2.0/AnimatedMorphCube/glTF/AnimatedMorphCube.gltf",
                "data/glTF-Sample-Models/2.0/AnimatedMorphSphere/glTF/AnimatedMorphSphere.gltf",
                "data/glTF-Sample-Models/2.0/AnimatedTriangle/glTF/AnimatedTriangle.gltf",
                "data/glTF-Sample-Models/2.0/Avocado/glTF/Avocado.gltf",
                "data/glTF-Sample-Models/2.0/BarramundiFish/glTF/BarramundiFish.gltf",
                "data/glTF-Sample-Models/2.0/BoomBox/glTF/BoomBox.gltf",
                "data/glTF-Sample-Models/2.0/BoomBoxWithAxes/glTF/BoomBoxWithAxes.gltf",
                "data/glTF-Sample-Models/2.0/Box/glTF/Box.gltf",
                "data/glTF-Sample-Models/2.0/BoxAnimated/glTF/BoxAnimated.gltf",
                "data/glTF-Sample-Models/2.0/BoxInterleaved/glTF/BoxInterleaved.gltf",
                "data/glTF-Sample-Models/2.0/BoxTextured/glTF/BoxTextured.gltf",
                "data/glTF-Sample-Models/2.0/BoxTexturedNonPowerOfTwo/glTF/BoxTexturedNonPowerOfTwo.gltf",
                "data/glTF-Sample-Models/2.0/BoxVertexColors/glTF/BoxVertexColors.gltf",
                "data/glTF-Sample-Models/2.0/BrainStem/glTF/BrainStem.gltf",
                "data/glTF-Sample-Models/2.0/Buggy/glTF/Buggy.gltf",
                "data/glTF-Sample-Models/2.0/Cameras/glTF/Cameras.gltf",
                "data/glTF-Sample-Models/2.0/CesiumMan/glTF/CesiumMan.gltf",
                "data/glTF-Sample-Models/2.0/CesiumMilkTruck/glTF/CesiumMilkTruck.gltf",
                "data/glTF-Sample-Models/2.0/Corset/glTF/Corset.gltf",
                "data/glTF-Sample-Models/2.0/Cube/glTF/Cube.gltf",
                "data/glTF-Sample-Models/2.0/DamagedHelmet/glTF/DamagedHelmet.gltf",
                "data/glTF-Sample-Models/2.0/Duck/glTF/Duck.gltf",
                "data/glTF-Sample-Models/2.0/FlightHelmet/glTF/FlightHelmet.gltf",
                "data/glTF-Sample-Models/2.0/GearboxAssy/glTF/GearboxAssy.gltf",
                "data/glTF-Sample-Models/2.0/Lantern/glTF/Lantern.gltf",
                "data/glTF-Sample-Models/2.0/MetalRoughSpheres/glTF/MetalRoughSpheres.gltf",
                "data/glTF-Sample-Models/2.0/Monster/glTF/Monster.gltf",
                "data/glTF-Sample-Models/2.0/NormalTangentMirrorTest/glTF/NormalTangentMirrorTest.gltf",
                "data/glTF-Sample-Models/2.0/NormalTangentTest/glTF/NormalTangentTest.gltf",
                "data/glTF-Sample-Models/2.0/OrientationTest/glTF/OrientationTest.gltf",
                "data/glTF-Sample-Models/2.0/ReciprocatingSaw/glTF/ReciprocatingSaw.gltf",
                "data/glTF-Sample-Models/2.0/RiggedFigure/glTF/RiggedFigure.gltf",
                "data/glTF-Sample-Models/2.0/RiggedSimple/glTF/RiggedSimple.gltf",
                "data/glTF-Sample-Models/2.0/SciFiHelmet/glTF/SciFiHelmet.gltf",
                "data/glTF-Sample-Models/2.0/SimpleMeshes/glTF/SimpleMeshes.gltf",
                "data/glTF-Sample-Models/2.0/SimpleMorph/glTF/SimpleMorph.gltf",
                "data/glTF-Sample-Models/2.0/SimpleSparseAccessor/glTF/SimpleSparseAccessor.gltf",
                "data/glTF-Sample-Models/2.0/Suzanne/glTF/Suzanne.gltf",
                "data/glTF-Sample-Models/2.0/TextureCoordinateTest/glTF/TextureCoordinateTest.gltf",
                "data/glTF-Sample-Models/2.0/TextureSettingsTest/glTF/TextureSettingsTest.gltf",
                "data/glTF-Sample-Models/2.0/Triangle/glTF/Triangle.gltf",
                "data/glTF-Sample-Models/2.0/TriangleWithoutIndices/glTF/TriangleWithoutIndices.gltf",
                "data/glTF-Sample-Models/2.0/TwoSidedPlane/glTF/TwoSidedPlane.gltf",
                "data/glTF-Sample-Models/2.0/VC/glTF/VC.gltf",
                "data/glTF-Sample-Models/2.0/VertexColorTest/glTF/VertexColorTest.gltf",
                "data/glTF-Sample-Models/2.0/WaterBottle/glTF/WaterBottle.gltf"
            })
        {
            RoundtripCompare(filePath);
        }
    }

    SECTION("roundtrip - .gltf files w/embedded resources")
    {
        for (auto & filePath :
            {
                "data/glTF-Sample-Models/2.0/2CylinderEngine/glTF-Embedded/2CylinderEngine.gltf",
                "data/glTF-Sample-Models/2.0/AlphaBlendModeTest/glTF-Embedded/AlphaBlendModeTest.gltf",
                "data/glTF-Sample-Models/2.0/AnimatedTriangle/glTF-Embedded/AnimatedTriangle.gltf",
                "data/glTF-Sample-Models/2.0/Box/glTF-Embedded/Box.gltf",
                "data/glTF-Sample-Models/2.0/BoxAnimated/glTF-Embedded/BoxAnimated.gltf",
                "data/glTF-Sample-Models/2.0/BoxInterleaved/glTF-Embedded/BoxInterleaved.gltf",
                "data/glTF-Sample-Models/2.0/BoxTextured/glTF-Embedded/BoxTextured.gltf",
                "data/glTF-Sample-Models/2.0/BoxTexturedNonPowerOfTwo/glTF-Embedded/BoxTexturedNonPowerOfTwo.gltf",
                "data/glTF-Sample-Models/2.0/BoxVertexColors/glTF-Embedded/BoxVertexColors.gltf",
                "data/glTF-Sample-Models/2.0/BrainStem/glTF-Embedded/BrainStem.gltf",
                "data/glTF-Sample-Models/2.0/Buggy/glTF-Embedded/Buggy.gltf",
                "data/glTF-Sample-Models/2.0/Cameras/glTF-Embedded/Cameras.gltf",
                "data/glTF-Sample-Models/2.0/CesiumMan/glTF-Embedded/CesiumMan.gltf",
                "data/glTF-Sample-Models/2.0/CesiumMilkTruck/glTF-Embedded/CesiumMilkTruck.gltf",
                "data/glTF-Sample-Models/2.0/DamagedHelmet/glTF-Embedded/DamagedHelmet.gltf",
                "data/glTF-Sample-Models/2.0/Duck/glTF-Embedded/Duck.gltf",
                "data/glTF-Sample-Models/2.0/GearboxAssy/glTF-Embedded/GearboxAssy.gltf",
                "data/glTF-Sample-Models/2.0/MetalRoughSpheres/glTF-Embedded/MetalRoughSpheres.gltf",
                "data/glTF-Sample-Models/2.0/Monster/glTF-Embedded/Monster.gltf",
                "data/glTF-Sample-Models/2.0/NormalTangentMirrorTest/glTF-Embedded/NormalTangentMirrorTest.gltf",
                "data/glTF-Sample-Models/2.0/NormalTangentTest/glTF-Embedded/NormalTangentTest.gltf",
                "data/glTF-Sample-Models/2.0/OrientationTest/glTF-Embedded/OrientationTest.gltf",
                "data/glTF-Sample-Models/2.0/ReciprocatingSaw/glTF-Embedded/ReciprocatingSaw.gltf",
                "data/glTF-Sample-Models/2.0/RiggedFigure/glTF-Embedded/RiggedFigure.gltf",
                "data/glTF-Sample-Models/2.0/RiggedSimple/glTF-Embedded/RiggedSimple.gltf",
                "data/glTF-Sample-Models/2.0/SimpleMeshes/glTF-Embedded/SimpleMeshes.gltf",
                //"data/glTF-Sample-Models/2.0/SimpleMorph/glTF-Embedded/SimpleMorph.gltf",
                //"data/glTF-Sample-Models/2.0/SimpleSparseAccessor/glTF-Embedded/SimpleSparseAccessor.gltf",
                "data/glTF-Sample-Models/2.0/TextureCoordinateTest/glTF-Embedded/TextureCoordinateTest.gltf",
                "data/glTF-Sample-Models/2.0/TextureSettingsTest/glTF-Embedded/TextureSettingsTest.gltf",
                "data/glTF-Sample-Models/2.0/Triangle/glTF-Embedded/Triangle.gltf",
                "data/glTF-Sample-Models/2.0/TriangleWithoutIndices/glTF-Embedded/TriangleWithoutIndices.gltf",
                "data/glTF-Sample-Models/2.0/VC/glTF-Embedded/VC.gltf",
                "data/glTF-Sample-Models/2.0/VertexColorTest/glTF-Embedded/VertexColorTest.gltf"
            })
        {
            RoundtripCompare(filePath);
        }
    }

    SECTION("load - .glb files")
    {
        for (auto & filePath :
            {
                "data/glTF-Sample-Models/2.0/2CylinderEngine/glTF-Binary/2CylinderEngine.glb",
                "data/glTF-Sample-Models/2.0/AlphaBlendModeTest/glTF-Binary/AlphaBlendModeTest.glb",
                "data/glTF-Sample-Models/2.0/AnimatedMorphCube/glTF-Binary/AnimatedMorphCube.glb",
                "data/glTF-Sample-Models/2.0/AnimatedMorphSphere/glTF-Binary/AnimatedMorphSphere.glb",
                "data/glTF-Sample-Models/2.0/Avocado/glTF-Binary/Avocado.glb",
                "data/glTF-Sample-Models/2.0/BarramundiFish/glTF-Binary/BarramundiFish.glb",
                "data/glTF-Sample-Models/2.0/BoomBox/glTF-Binary/BoomBox.glb",
                "data/glTF-Sample-Models/2.0/Box/glTF-Binary/Box.glb",
                "data/glTF-Sample-Models/2.0/BoxAnimated/glTF-Binary/BoxAnimated.glb",
                "data/glTF-Sample-Models/2.0/BoxInterleaved/glTF-Binary/BoxInterleaved.glb",
                "data/glTF-Sample-Models/2.0/BoxTextured/glTF-Binary/BoxTextured.glb",
                "data/glTF-Sample-Models/2.0/BoxTexturedNonPowerOfTwo/glTF-Binary/BoxTexturedNonPowerOfTwo.glb",
                "data/glTF-Sample-Models/2.0/BoxVertexColors/glTF-Binary/BoxVertexColors.glb",
                "data/glTF-Sample-Models/2.0/BrainStem/glTF-Binary/BrainStem.glb",
                "data/glTF-Sample-Models/2.0/Buggy/glTF-Binary/Buggy.glb",
                "data/glTF-Sample-Models/2.0/CesiumMan/glTF-Binary/CesiumMan.glb",
                "data/glTF-Sample-Models/2.0/CesiumMilkTruck/glTF-Binary/CesiumMilkTruck.glb",
                "data/glTF-Sample-Models/2.0/Corset/glTF-Binary/Corset.glb",
                "data/glTF-Sample-Models/2.0/DamagedHelmet/glTF-Binary/DamagedHelmet.glb",
                "data/glTF-Sample-Models/2.0/Duck/glTF-Binary/Duck.glb",
                "data/glTF-Sample-Models/2.0/GearboxAssy/glTF-Binary/GearboxAssy.glb",
                "data/glTF-Sample-Models/2.0/Lantern/glTF-Binary/Lantern.glb",
                "data/glTF-Sample-Models/2.0/MetalRoughSpheres/glTF-Binary/MetalRoughSpheres.glb",
                "data/glTF-Sample-Models/2.0/Monster/glTF-Binary/Monster.glb",
                "data/glTF-Sample-Models/2.0/NormalTangentMirrorTest/glTF-Binary/NormalTangentMirrorTest.glb",
                "data/glTF-Sample-Models/2.0/NormalTangentTest/glTF-Binary/NormalTangentTest.glb",
                "data/glTF-Sample-Models/2.0/OrientationTest/glTF-Binary/OrientationTest.glb",
                "data/glTF-Sample-Models/2.0/ReciprocatingSaw/glTF-Binary/ReciprocatingSaw.glb",
                "data/glTF-Sample-Models/2.0/RiggedFigure/glTF-Binary/RiggedFigure.glb",
                "data/glTF-Sample-Models/2.0/RiggedSimple/glTF-Binary/RiggedSimple.glb",
                "data/glTF-Sample-Models/2.0/TextureCoordinateTest/glTF-Binary/TextureCoordinateTest.glb",
                "data/glTF-Sample-Models/2.0/TextureSettingsTest/glTF-Binary/TextureSettingsTest.glb",
                "data/glTF-Sample-Models/2.0/VC/glTF-Binary/VC.glb",
                "data/glTF-Sample-Models/2.0/VertexColorTest/glTF-Binary/VertexColorTest.glb",
                "data/glTF-Sample-Models/2.0/WaterBottle/glTF-Binary/WaterBottle.glb"
            })
        {
            CheckBinary(filePath);
        }
    }
}
