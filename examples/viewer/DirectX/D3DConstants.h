// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <DirectXMath.h>

struct SceneConstantBuffer
{
    DirectX::XMVECTOR eye;

    DirectX::XMVECTOR lightDir[2];
    DirectX::XMVECTOR lightColor[2];
};

struct MeshConstantBuffer
{
    DirectX::XMMATRIX worldViewProj;
    DirectX::XMMATRIX world;

    int materialIndex;
};

struct MeshShaderData
{
    DirectX::XMFLOAT4 meshAutoColor{};

    int baseColorIndex{};
    DirectX::XMFLOAT4 baseColorFactor{};

    int normalIndex{};
    float normalScale{};

    int metalRoughIndex{};
    float roughnessFactor{};
    float metallicFactor{};

    int aoIndex{};
    float aoStrength{};

    int emissiveIndex{};
    DirectX::XMFLOAT3 emissiveFactor{};
};