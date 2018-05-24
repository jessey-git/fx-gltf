// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <unordered_map>
#include <wrl.h>
#include "D3DFrameResources.h"
#include "ShaderOptions.h"

struct D3DRenderContext
{
    ID3D12GraphicsCommandList * commandList;
    D3DFrameResource const & currentFrame;
    DirectX::CXMMATRIX viewProj;
    std::size_t currentCBIndex;
    ShaderOptions currentShaderOptions;
    ShaderOptions overrideShaderOptions;
    std::unordered_map<ShaderOptions, Microsoft::WRL::ComPtr<ID3D12PipelineState>> & pipelineStateMap;
};