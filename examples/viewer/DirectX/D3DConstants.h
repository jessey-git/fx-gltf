// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <DirectXMath.h>

struct SceneConstantBuffer
{
    DirectX::XMVECTOR lightDir[2];
    DirectX::XMVECTOR lightColor[2];
};

struct MeshConstantBuffer
{
    DirectX::XMMATRIX worldViewProj;
    DirectX::XMMATRIX world;
    DirectX::XMFLOAT3 meshColor;
};
