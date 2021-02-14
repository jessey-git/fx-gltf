// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------

#include <catch2/catch.hpp>
#include <fx/gltf.h>
#include <nlohmann/json.hpp>
#include <string>

#include "utility.h"

bool FindExtension(std::string const & extensionName, nlohmann::json const & json)
{
    const nlohmann::json::const_iterator iterExtensions = json.find("extensions");
    if (iterExtensions != json.end())
    {
        nlohmann::json const & ext = *iterExtensions;
        return ext.find(extensionName) != ext.end();
    }

    return false;
}

void RoundtripCompare(std::string const & filePath, bool checkExtensions = false)
{
    CAPTURE(filePath);

    std::string errorString = "Failed: ";
    errorString.append(filePath).append("\n");

    try
    {
        fx::gltf::Document doc = fx::gltf::LoadFromText(filePath);

        nlohmann::json current = doc;
        nlohmann::json original = utility::LoadJsonFromFile(filePath);

        nlohmann::json filteredDiff = utility::DiffAndFilter(current, original);
        if (!filteredDiff.empty())
        {
            throw std::runtime_error(errorString.append(filteredDiff.dump(2)));
        }

        // Ensure all the buffers have data...
        for (auto const & b : doc.buffers)
        {
            REQUIRE(b.data.size() == b.byteLength);
        }

        // Ensure used extensions are actually still referenced...
        if (checkExtensions)
        {
            for (auto & e : doc.extensionsUsed)
            {
                bool found = false;
                for (auto & m : doc.meshes)
                {
                    for (auto & p : m.primitives)
                    {
                        found |= FindExtension(e, p.extensionsAndExtras);
                        if (found)
                        {
                            break;
                        }
                    }
                }

                REQUIRE(found);
            }
        }
    }
    catch (fx::gltf::invalid_gltf_document & e)
    {
        fx::FormatException(errorString, e);

        throw std::runtime_error(errorString);
    }
}

void CheckBinary(std::string const & filePath)
{
    CAPTURE(filePath);

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
        fx::FormatException(errorString, e);

        throw std::runtime_error(errorString);
    }
}

TEST_CASE("roundtrip")
{
    SECTION("roundtrip - .gltf files w/external resources")
    {
        for (auto const & filePath :
             {
                 "data/glTF-Sample-Models/2.0/2CylinderEngine/glTF/2CylinderEngine.gltf",
                 "data/glTF-Sample-Models/2.0/AlphaBlendModeTest/glTF/AlphaBlendModeTest.gltf",
                 "data/glTF-Sample-Models/2.0/AnimatedCube/glTF/AnimatedCube.gltf",
                 "data/glTF-Sample-Models/2.0/AnimatedMorphCube/glTF/AnimatedMorphCube.gltf",
                 "data/glTF-Sample-Models/2.0/AnimatedMorphSphere/glTF/AnimatedMorphSphere.gltf",
                 "data/glTF-Sample-Models/2.0/AnimatedTriangle/glTF/AnimatedTriangle.gltf",
                 "data/glTF-Sample-Models/2.0/AntiqueCamera/glTF/AntiqueCamera.gltf",
                 "data/glTF-Sample-Models/2.0/Avocado/glTF/Avocado.gltf",
                 "data/glTF-Sample-Models/2.0/BarramundiFish/glTF/BarramundiFish.gltf",
                 "data/glTF-Sample-Models/2.0/BoomBox/glTF/BoomBox.gltf",
                 "data/glTF-Sample-Models/2.0/BoomBoxWithAxes/glTF/BoomBoxWithAxes.gltf",
                 "data/glTF-Sample-Models/2.0/Box/glTF/Box.gltf",
                 "data/glTF-Sample-Models/2.0/Box With Spaces/glTF/Box With Spaces.gltf",
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
                 "data/glTF-Sample-Models/2.0/ClearCoatTest/glTF/ClearCoatTest.gltf",
                 "data/glTF-Sample-Models/2.0/Corset/glTF/Corset.gltf",
                 "data/glTF-Sample-Models/2.0/Cube/glTF/Cube.gltf",
                 "data/glTF-Sample-Models/2.0/DamagedHelmet/glTF/DamagedHelmet.gltf",
                 "data/glTF-Sample-Models/2.0/Duck/glTF/Duck.gltf",
                 "data/glTF-Sample-Models/2.0/EnvironmentTest/glTF/EnvironmentTest.gltf",
                 "data/glTF-Sample-Models/2.0/FlightHelmet/glTF/FlightHelmet.gltf",
                 "data/glTF-Sample-Models/2.0/Fox/glTF/Fox.gltf",
                 "data/glTF-Sample-Models/2.0/GearboxAssy/glTF/GearboxAssy.gltf",
                 "data/glTF-Sample-Models/2.0/InterpolationTest/glTF/InterpolationTest.gltf",
                 "data/glTF-Sample-Models/2.0/Lantern/glTF/Lantern.gltf",
                 "data/glTF-Sample-Models/2.0/MaterialsVariantsShoe/glTF/MaterialsVariantsShoe.gltf",
                 "data/glTF-Sample-Models/2.0/MetalRoughSpheres/glTF/MetalRoughSpheres.gltf",
                 "data/glTF-Sample-Models/2.0/MetalRoughSpheresNoTextures/glTF/MetalRoughSpheresNoTextures.gltf",
                 "data/glTF-Sample-Models/2.0/MorphPrimitivesTest/glTF/MorphPrimitivesTest.gltf",
                 "data/glTF-Sample-Models/2.0/MultiUVTest/glTF/MultiUVTest.gltf",
                 "data/glTF-Sample-Models/2.0/NormalTangentMirrorTest/glTF/NormalTangentMirrorTest.gltf",
                 "data/glTF-Sample-Models/2.0/NormalTangentTest/glTF/NormalTangentTest.gltf",
                 "data/glTF-Sample-Models/2.0/OrientationTest/glTF/OrientationTest.gltf",
                 "data/glTF-Sample-Models/2.0/ReciprocatingSaw/glTF/ReciprocatingSaw.gltf",
                 "data/glTF-Sample-Models/2.0/RiggedFigure/glTF/RiggedFigure.gltf",
                 "data/glTF-Sample-Models/2.0/RiggedSimple/glTF/RiggedSimple.gltf",
                 "data/glTF-Sample-Models/2.0/SciFiHelmet/glTF/SciFiHelmet.gltf",
                 "data/glTF-Sample-Models/2.0/SheenChair/glTF/SheenChair.gltf",
                 "data/glTF-Sample-Models/2.0/SheenCloth/glTF/SheenCloth.gltf",
                 "data/glTF-Sample-Models/2.0/SimpleMeshes/glTF/SimpleMeshes.gltf",
                 "data/glTF-Sample-Models/2.0/SimpleMorph/glTF/SimpleMorph.gltf",
                 "data/glTF-Sample-Models/2.0/SimpleSkin/glTF/SimpleSkin.gltf",
                 "data/glTF-Sample-Models/2.0/SimpleSparseAccessor/glTF/SimpleSparseAccessor.gltf",
                 "data/glTF-Sample-Models/2.0/SpecGlossVsMetalRough/glTF/SpecGlossVsMetalRough.gltf",
                 "data/glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf",
                 "data/glTF-Sample-Models/2.0/Suzanne/glTF/Suzanne.gltf",
                 "data/glTF-Sample-Models/2.0/TextureCoordinateTest/glTF/TextureCoordinateTest.gltf",
                 "data/glTF-Sample-Models/2.0/TextureSettingsTest/glTF/TextureSettingsTest.gltf",
                 "data/glTF-Sample-Models/2.0/TextureTransformMultiTest/glTF/TextureTransformMultiTest.gltf",
                 "data/glTF-Sample-Models/2.0/TextureTransformTest/glTF/TextureTransformTest.gltf",
                 "data/glTF-Sample-Models/2.0/ToyCar/glTF/ToyCar.gltf",
                 "data/glTF-Sample-Models/2.0/TransmissionTest/glTF/TransmissionTest.gltf",
                 "data/glTF-Sample-Models/2.0/Triangle/glTF/Triangle.gltf",
                 "data/glTF-Sample-Models/2.0/TriangleWithoutIndices/glTF/TriangleWithoutIndices.gltf",
                 "data/glTF-Sample-Models/2.0/TwoSidedPlane/glTF/TwoSidedPlane.gltf",
                 "data/glTF-Sample-Models/2.0/UnlitTest/glTF/UnlitTest.gltf",
                 "data/glTF-Sample-Models/2.0/VC/glTF/VC.gltf",
                 "data/glTF-Sample-Models/2.0/VertexColorTest/glTF/VertexColorTest.gltf",
                 "data/glTF-Sample-Models/2.0/WaterBottle/glTF/WaterBottle.gltf" })
        {
            RoundtripCompare(filePath);
        }
    }

    SECTION("roundtrip - .gltf files w/embedded resources")
    {
        for (auto const & filePath :
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
                 "data/glTF-Sample-Models/2.0/Fox/glTF-Embedded/Fox.gltf",
                 "data/glTF-Sample-Models/2.0/GearboxAssy/glTF-Embedded/GearboxAssy.gltf",
                 "data/glTF-Sample-Models/2.0/MetalRoughSpheres/glTF-Embedded/MetalRoughSpheres.gltf",
                 "data/glTF-Sample-Models/2.0/MultiUVTest/glTF-Embedded/MultiUVTest.gltf",
                 "data/glTF-Sample-Models/2.0/NormalTangentMirrorTest/glTF-Embedded/NormalTangentMirrorTest.gltf",
                 "data/glTF-Sample-Models/2.0/NormalTangentTest/glTF-Embedded/NormalTangentTest.gltf",
                 "data/glTF-Sample-Models/2.0/OrientationTest/glTF-Embedded/OrientationTest.gltf",
                 "data/glTF-Sample-Models/2.0/ReciprocatingSaw/glTF-Embedded/ReciprocatingSaw.gltf",
                 "data/glTF-Sample-Models/2.0/RiggedFigure/glTF-Embedded/RiggedFigure.gltf",
                 "data/glTF-Sample-Models/2.0/RiggedSimple/glTF-Embedded/RiggedSimple.gltf",
                 "data/glTF-Sample-Models/2.0/SimpleMeshes/glTF-Embedded/SimpleMeshes.gltf",
                 "data/glTF-Sample-Models/2.0/SimpleMorph/glTF-Embedded/SimpleMorph.gltf",
                 "data/glTF-Sample-Models/2.0/SimpleSkin/glTF-Embedded/SimpleSkin.gltf",
                 "data/glTF-Sample-Models/2.0/SimpleSparseAccessor/glTF-Embedded/SimpleSparseAccessor.gltf",
                 "data/glTF-Sample-Models/2.0/TextureCoordinateTest/glTF-Embedded/TextureCoordinateTest.gltf",
                 "data/glTF-Sample-Models/2.0/TextureSettingsTest/glTF-Embedded/TextureSettingsTest.gltf",
                 "data/glTF-Sample-Models/2.0/Triangle/glTF-Embedded/Triangle.gltf",
                 "data/glTF-Sample-Models/2.0/TriangleWithoutIndices/glTF-Embedded/TriangleWithoutIndices.gltf",
                 "data/glTF-Sample-Models/2.0/VC/glTF-Embedded/VC.gltf",
                 "data/glTF-Sample-Models/2.0/VertexColorTest/glTF-Embedded/VertexColorTest.gltf" })
        {
            RoundtripCompare(filePath);
        }
    }

    SECTION("roundtrip - .gltf files w/extensions")
    {
        for (auto const & filePath :
             {
                 "data/glTF-Sample-Models/2.0/2CylinderEngine/glTF-Draco/2CylinderEngine.gltf",
                 "data/glTF-Sample-Models/2.0/Avocado/glTF-Draco/Avocado.gltf",
                 "data/glTF-Sample-Models/2.0/BarramundiFish/glTF-Draco/BarramundiFish.gltf",
                 "data/glTF-Sample-Models/2.0/BoomBox/glTF-Draco/BoomBox.gltf",
                 "data/glTF-Sample-Models/2.0/Box/glTF-Draco/Box.gltf",
                 "data/glTF-Sample-Models/2.0/BrainStem/glTF-Draco/BrainStem.gltf",
                 "data/glTF-Sample-Models/2.0/Buggy/glTF-Draco/Buggy.gltf",
                 "data/glTF-Sample-Models/2.0/CesiumMan/glTF-Draco/CesiumMan.gltf",
                 "data/glTF-Sample-Models/2.0/CesiumMilkTruck/glTF-Draco/CesiumMilkTruck.gltf",
                 "data/glTF-Sample-Models/2.0/Corset/glTF-Draco/Corset.gltf",
                 "data/glTF-Sample-Models/2.0/Duck/glTF-Draco/Duck.gltf",
                 "data/glTF-Sample-Models/2.0/GearboxAssy/glTF-Draco/GearboxAssy.gltf",
                 "data/glTF-Sample-Models/2.0/Lantern/glTF-Draco/Lantern.gltf",
                 "data/glTF-Sample-Models/2.0/MorphPrimitivesTest/glTF-Draco/MorphPrimitivesTest.gltf",
                 "data/glTF-Sample-Models/2.0/ReciprocatingSaw/glTF-Draco/ReciprocatingSaw.gltf",
                 "data/glTF-Sample-Models/2.0/RiggedFigure/glTF-Draco/RiggedFigure.gltf",
                 "data/glTF-Sample-Models/2.0/RiggedSimple/glTF-Draco/RiggedSimple.gltf",
                 "data/glTF-Sample-Models/2.0/VC/glTF-Draco/VC.gltf",
                 "data/glTF-Sample-Models/2.0/WaterBottle/glTF-Draco/WaterBottle.gltf" })
        {
            RoundtripCompare(filePath, true);
        }
    }

    SECTION("load - .glb files")
    {
        for (auto const & filePath :
             {
                 "data/glTF-Sample-Models/2.0/2CylinderEngine/glTF-Binary/2CylinderEngine.glb",
                 "data/glTF-Sample-Models/2.0/AlphaBlendModeTest/glTF-Binary/AlphaBlendModeTest.glb",
                 "data/glTF-Sample-Models/2.0/AnimatedMorphCube/glTF-Binary/AnimatedMorphCube.glb",
                 "data/glTF-Sample-Models/2.0/AnimatedMorphSphere/glTF-Binary/AnimatedMorphSphere.glb",
                 "data/glTF-Sample-Models/2.0/AntiqueCamera/glTF-Binary/AntiqueCamera.glb",
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
                 "data/glTF-Sample-Models/2.0/ClearCoatTest/glTF-Binary/ClearCoatTest.glb",
                 "data/glTF-Sample-Models/2.0/Corset/glTF-Binary/Corset.glb",
                 "data/glTF-Sample-Models/2.0/DamagedHelmet/glTF-Binary/DamagedHelmet.glb",
                 "data/glTF-Sample-Models/2.0/Duck/glTF-Binary/Duck.glb",
                 "data/glTF-Sample-Models/2.0/Fox/glTF-Binary/Fox.glb",
                 "data/glTF-Sample-Models/2.0/GearboxAssy/glTF-Binary/GearboxAssy.glb",
                 "data/glTF-Sample-Models/2.0/InterpolationTest/glTF-Binary/InterpolationTest.glb",
                 "data/glTF-Sample-Models/2.0/Lantern/glTF-Binary/Lantern.glb",
                 "data/glTF-Sample-Models/2.0/MaterialsVariantsShoe/glTF-Binary/MaterialsVariantsShoe.glb",
                 "data/glTF-Sample-Models/2.0/MetalRoughSpheres/glTF-Binary/MetalRoughSpheres.glb",
                 "data/glTF-Sample-Models/2.0/MetalRoughSpheresNoTextures/glTF-Binary/MetalRoughSpheresNoTextures.glb",
                 "data/glTF-Sample-Models/2.0/MorphPrimitivesTest/glTF-Binary/MorphPrimitivesTest.glb",
                 "data/glTF-Sample-Models/2.0/MultiUVTest/glTF-Binary/MultiUVTest.glb",
                 "data/glTF-Sample-Models/2.0/NormalTangentMirrorTest/glTF-Binary/NormalTangentMirrorTest.glb",
                 "data/glTF-Sample-Models/2.0/NormalTangentTest/glTF-Binary/NormalTangentTest.glb",
                 "data/glTF-Sample-Models/2.0/OrientationTest/glTF-Binary/OrientationTest.glb",
                 "data/glTF-Sample-Models/2.0/ReciprocatingSaw/glTF-Binary/ReciprocatingSaw.glb",
                 "data/glTF-Sample-Models/2.0/RiggedFigure/glTF-Binary/RiggedFigure.glb",
                 "data/glTF-Sample-Models/2.0/RiggedSimple/glTF-Binary/RiggedSimple.glb",
                 "data/glTF-Sample-Models/2.0/SheenChair/glTF-Binary/SheenChair.glb",
                 "data/glTF-Sample-Models/2.0/SpecGlossVsMetalRough/glTF-Binary/SpecGlossVsMetalRough.glb",
                 "data/glTF-Sample-Models/2.0/TextureCoordinateTest/glTF-Binary/TextureCoordinateTest.glb",
                 "data/glTF-Sample-Models/2.0/TextureSettingsTest/glTF-Binary/TextureSettingsTest.glb",
                 "data/glTF-Sample-Models/2.0/TextureTransformMultiTest/glTF-Binary/TextureTransformMultiTest.glb",
                 "data/glTF-Sample-Models/2.0/ToyCar/glTF-Binary/ToyCar.glb",
                 "data/glTF-Sample-Models/2.0/TransmissionTest/glTF-Binary/TransmissionTest.glb",
                 "data/glTF-Sample-Models/2.0/UnlitTest/glTF-Binary/UnlitTest.glb",
                 "data/glTF-Sample-Models/2.0/VC/glTF-Binary/VC.glb",
                 "data/glTF-Sample-Models/2.0/VertexColorTest/glTF-Binary/VertexColorTest.glb",
                 "data/glTF-Sample-Models/2.0/WaterBottle/glTF-Binary/WaterBottle.glb" })
        {
            CheckBinary(filePath);
        }
    }
}
