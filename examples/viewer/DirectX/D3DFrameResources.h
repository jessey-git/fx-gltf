// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include "D3DConstants.h"
#include "D3DUploadBuffer.h"

struct D3DFrameResource
{
    D3DFrameResource(ID3D12Device* device, UINT passCount, UINT objectCount);
    D3DFrameResource(const D3DFrameResource& rhs) = delete;
    D3DFrameResource& operator=(const D3DFrameResource& rhs) = delete;
    ~D3DFrameResource() = default;

    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

    std::unique_ptr<D3DUploadBuffer<SceneConstantBuffer>> PassCB = nullptr;
    std::unique_ptr<D3DUploadBuffer<MeshConstantBuffer>> ObjectCB = nullptr;

    UINT64 Fence = 0;
};