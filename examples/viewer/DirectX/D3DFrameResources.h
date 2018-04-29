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
    explicit D3DFrameResource(ID3D12Device * device);

    D3DFrameResource(D3DFrameResource const & rhs) = delete;
    D3DFrameResource(D3DFrameResource && rhs) = default;

    D3DFrameResource & operator=(const D3DFrameResource & rhs) = delete;
    D3DFrameResource & operator=(D3DFrameResource && rhs) = delete;
    ~D3DFrameResource() = default;

    void AllocateConstantBuffers(ID3D12Device * device, UINT sceneCount, UINT meshCount);

    // clang-format off
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>          CommandAllocator{};
    Microsoft::WRL::ComPtr<ID3D12Resource>                  RenderTarget{};
    UINT64                                                  Fence{};

    std::unique_ptr<D3DUploadBuffer<SceneConstantBuffer>>   SceneCB{};
    std::unique_ptr<D3DUploadBuffer<MeshConstantBuffer>>    MeshCB{};
    // clang-format on
};
