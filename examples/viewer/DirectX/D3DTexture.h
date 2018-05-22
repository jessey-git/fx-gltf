// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <fx/gltf.h>
#include <vector>

#include "D3DDeviceResources.h"
#include "D3DUtil.h"

class D3DTexture
{
public:
    void Create(std::string const & texture, DX::D3DDeviceResources const * deviceResources);

    void CreateSRV(ID3D12Device * device, CD3DX12_CPU_DESCRIPTOR_HANDLE const & descriptor);

    void FinishUpload();

private:
    struct D3DTextureData
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> m_mainBuffer{};
        Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadBuffer{};
    };

    D3DTextureData m_data{};
};
