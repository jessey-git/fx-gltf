// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include "D3DDeviceResources.h"
#include "ImageData.h"

class D3DTexture
{
public:
    void Create(ImageData const & texture, D3DDeviceResources const * deviceResources);

    void CreateSRV(ID3D12Device * device, CD3DX12_CPU_DESCRIPTOR_HANDLE const & descriptor);

    void FinishUpload();

private:
    struct D3DTextureData
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> DefaultBuffer{};
        Microsoft::WRL::ComPtr<ID3D12Resource> UploadBuffer{};
    };

    D3DTextureData m_data{};
};
