// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <DirectXMath.h>

// The odd ordering of fields is to keep alignment correct
struct Light
{
    DirectX::XMFLOAT3 Strength{ 0.5f, 0.5f, 0.5f };
    float FalloffStart{ 1.0f };                         // point/spot light only
    DirectX::XMFLOAT3 Direction{ 0.0f, -1.0f, 0.0f };   // directional/spot light only
    float FalloffEnd{ 10.0f };                          // point/spot light only
    DirectX::XMFLOAT3 Position{ 0.0f, 0.0f, 0.0f };     // point/spot light only
    float SpotPower{ 64.0f };                           // spot light only
};

struct SceneConstantBuffer
{
    DirectX::XMMATRIX ViewProj{};
    DirectX::XMVECTOR Camera{};

    Light DirectionalLight{};
    Light PointLights[2]{};
};

struct MeshConstantBuffer
{
    DirectX::XMMATRIX WorldViewProj{};
    DirectX::XMMATRIX World{};

    int MaterialIndex{};
};

struct MeshShaderData
{
    DirectX::XMFLOAT4 MeshAutoColor{};

    int BaseColorIndex{};
    DirectX::XMFLOAT4 BaseColorFactor{};

    int NormalIndex{};
    float NormalScale{};

    int MetalRoughIndex{};
    float RoughnessFactor{};
    float MetallicFactor{};

    int AOIndex{};
    float AOStrength{};

    int EmissiveIndex{};
    DirectX::XMFLOAT3 EmissiveFactor{};
};
