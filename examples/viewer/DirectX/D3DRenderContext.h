// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <DirectXMath.h>
#include <d3d12.h>
#include <unordered_map>
#include <wrl.h>
#include "D3DFrameResources.h"
#include "ShaderOptions.h"

struct D3DRenderContext
{
    ID3D12GraphicsCommandList * CommandList;
    D3DFrameResource const & CurrentFrame;
    DirectX::XMMATRIX const & ViewProj;
    std::size_t CurrentCBIndex;
    ShaderOptions CurrentShaderOptions;
    ShaderOptions OverrideShaderOptions;
    std::unordered_map<ShaderOptions, Microsoft::WRL::ComPtr<ID3D12PipelineState>> & PipelineStateMap;
};
