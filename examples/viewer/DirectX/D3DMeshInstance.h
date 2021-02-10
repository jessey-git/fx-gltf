// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <DirectXMath.h>

struct D3DMeshInstance
{
    DirectX::XMMATRIX Transform;

    uint32_t MeshIndex;
};
