// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#include "stdafx.h"
#include <fx/gltf.h>
#include <vector>

#include "D3DMesh.h"
#include "D3DUtil.h"
#include "MeshData.h"

void D3DMesh::CreateDeviceDependentResources(
    fx::gltf::Document const & doc, std::size_t meshIndex, std::unique_ptr<DX::D3DDeviceResources> const & deviceResources)
{
    ID3D12Device * device = deviceResources->GetD3DDevice();

    MeshData mesh(doc, meshIndex);

    const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    if (mesh.HasVertexData())
    {
        MeshData::BufferInfo buffer = mesh.VertexBuffer();
        const CD3DX12_RESOURCE_DESC resourceDescV = CD3DX12_RESOURCE_DESC::Buffer(buffer.totalSize);
        DX::ThrowIfFailed(device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDescV,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(m_vertexBuffer.ReleaseAndGetAddressOf())));

        // Copy the data to the vertex buffer.
        UINT8 * pVertexDataBegin;
        CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
        DX::ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void **>(&pVertexDataBegin)));
        std::memcpy(pVertexDataBegin, buffer.data, buffer.totalSize);
        m_vertexBuffer->Unmap(0, nullptr);

        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = buffer.dataStride;
        m_vertexBufferView.SizeInBytes = buffer.totalSize;
    }

    // Create the vertex buffer
    if (mesh.HasNormalData())
    {
        MeshData::BufferInfo buffer = mesh.NormalBuffer();

        const CD3DX12_RESOURCE_DESC resourceDescN = CD3DX12_RESOURCE_DESC::Buffer(buffer.totalSize);
        DX::ThrowIfFailed(device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDescN,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(m_normalBuffer.ReleaseAndGetAddressOf())));

        UINT8 * pVertexDataBegin;
        CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
        DX::ThrowIfFailed(m_normalBuffer->Map(0, &readRange, reinterpret_cast<void **>(&pVertexDataBegin)));
        std::memcpy(pVertexDataBegin, buffer.data, buffer.totalSize);
        m_normalBuffer->Unmap(0, nullptr);

        m_normalBufferView.BufferLocation = m_normalBuffer->GetGPUVirtualAddress();
        m_normalBufferView.StrideInBytes = buffer.dataStride;
        m_normalBufferView.SizeInBytes = buffer.totalSize;
    }

    // Create the index buffer
    if (mesh.HasIndexData())
    {
        MeshData::BufferInfo buffer = mesh.IndexBuffer();
        const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(buffer.totalSize);
        DX::ThrowIfFailed(device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(m_indexBuffer.ReleaseAndGetAddressOf())));

        UINT8 * pVertexDataBegin;
        CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
        DX::ThrowIfFailed(m_indexBuffer->Map(0, &readRange, reinterpret_cast<void **>(&pVertexDataBegin)));
        std::memcpy(pVertexDataBegin, buffer.data, buffer.totalSize);
        m_indexBuffer->Unmap(0, nullptr);

        // Initialize the index buffer view.
        m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
        m_indexBufferView.Format = GetFormat(buffer.accessor);
        m_indexBufferView.SizeInBytes = buffer.totalSize;

        m_indexCount = buffer.accessor->count;
    }
}

void D3DMesh::Render(ID3D12GraphicsCommandList * commandList)
{
    D3D12_VERTEX_BUFFER_VIEW * views[2] = { &m_vertexBufferView, &m_normalBufferView };

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 2, views[0]);
    commandList->IASetIndexBuffer(&m_indexBufferView);

    commandList->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
}

void D3DMesh::Rotate(float rotationAngleRad)
{
    XMStoreFloat4x4(&m_worldMatrix, DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationY(rotationAngleRad), DirectX::XMMatrixScaling(1, 1, 1)));
}

void D3DMesh::Reset()
{
    m_vertexBuffer.Reset();
    m_normalBuffer.Reset();
    m_indexBuffer.Reset();
}
