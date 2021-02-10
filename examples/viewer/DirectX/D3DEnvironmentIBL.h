// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include "D3DDeviceResources.h"

class D3DEnvironmentIBL
{
public:
    void Create(D3DDeviceResources const * deviceResources);

    void CreateSRV(ID3D12Device * device, CD3DX12_CPU_DESCRIPTOR_HANDLE & descriptor);

    void FinishUpload();

private:
    struct D3DEnvironmentData
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> DiffuseBuffer{};
        Microsoft::WRL::ComPtr<ID3D12Resource> SpecularBuffer{};
        Microsoft::WRL::ComPtr<ID3D12Resource> LUTBuffer{};

        Microsoft::WRL::ComPtr<ID3D12Resource> DiffuseUploadBuffer{};
        Microsoft::WRL::ComPtr<ID3D12Resource> SpecularUploadBuffer{};
        Microsoft::WRL::ComPtr<ID3D12Resource> LUTUploadBuffer{};
    };

    D3DEnvironmentData m_data{};
};
