// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#include "stdafx.h"

#include "D3DFrameResources.h"
#include "D3DUtil.h"

D3DFrameResource::D3DFrameResource(ID3D12Device * device)
{
    DX::ThrowIfFailed(device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(CommandAllocator.GetAddressOf())));
}

void D3DFrameResource::AllocateConstantBuffers(ID3D12Device * device, std::size_t sceneCount, std::size_t meshCount)
{
    SceneCB = std::make_unique<D3DUploadBuffer<SceneConstantBuffer>>(device, sceneCount, true);
    MeshCB = std::make_unique<D3DUploadBuffer<MeshConstantBuffer>>(device, meshCount, true);
}

void D3DFrameResource::Reset()
{
    CommandAllocator.Reset();
    RenderTarget.Reset();
    SceneCB->Reset();
    MeshCB->Reset();
}