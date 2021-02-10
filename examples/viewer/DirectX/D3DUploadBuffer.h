// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <d3d12.h>
#include "d3dx12.h"

#include "Platform/COMUtil.h"

template <typename T>
class D3DUploadBuffer
{
public:
    D3DUploadBuffer(ID3D12Device * device, std::size_t elementCount, bool isConstantBuffer)
    {
        m_elementByteSize = sizeof(T);
        if (isConstantBuffer)
        {
            m_elementByteSize = (sizeof(T) + 255u) & ~255u;
        }

        const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        const CD3DX12_RESOURCE_DESC resourceDescV = CD3DX12_RESOURCE_DESC::Buffer(m_elementByteSize * elementCount);
        COMUtil::ThrowIfFailed(device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDescV,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_uploadBuffer)));

        m_uploadBuffer->Map(0, nullptr, reinterpret_cast<void **>(&m_mappedData));
    }

    D3DUploadBuffer(D3DUploadBuffer const &) = delete;
    D3DUploadBuffer(D3DUploadBuffer &&) = delete;
    D3DUploadBuffer & operator=(D3DUploadBuffer const &) = delete;
    D3DUploadBuffer & operator=(D3DUploadBuffer &&) = delete;

    ~D3DUploadBuffer()
    {
        if (m_uploadBuffer != nullptr)
        {
            m_uploadBuffer->Unmap(0, nullptr);
        }

        m_mappedData = nullptr;
    }

    ID3D12Resource * Resource() const noexcept
    {
        return m_uploadBuffer.Get();
    }

    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress(std::size_t elementIndex)
    {
        return Resource()->GetGPUVirtualAddress() + (elementIndex * m_elementByteSize);
    }

    void CopyData(std::size_t elementIndex, T const & data) noexcept
    {
        std::memcpy(&m_mappedData[elementIndex * m_elementByteSize], &data, sizeof(T));
    }

    void Reset()
    {
        m_uploadBuffer.Reset();
    }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadBuffer{};
    uint8_t * m_mappedData{};

    uint32_t m_elementByteSize{};
};
