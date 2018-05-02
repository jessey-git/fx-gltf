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

uint32_t D3DMesh::CurrentMeshPartId = 1;

void D3DMesh::CreateDeviceDependentResources(
    fx::gltf::Document const & doc, std::size_t meshIndex, std::unique_ptr<DX::D3DDeviceResources> const & deviceResources)
{
    ID3D12Device * device = deviceResources->GetD3DDevice();

    const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    m_meshParts.resize(doc.meshes[meshIndex].primitives.size());
    for (std::size_t i = 0; i < doc.meshes[meshIndex].primitives.size(); i++)
    {
        MeshData mesh(doc, meshIndex, i);

        D3DMeshPart & meshPart = m_meshParts[i];
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
                IID_PPV_ARGS(meshPart.m_vertexBuffer.ReleaseAndGetAddressOf())));

            // Copy the data to the vertex buffer.
            UINT8 * pVertexDataBegin;
            CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
            DX::ThrowIfFailed(meshPart.m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void **>(&pVertexDataBegin)));
            std::memcpy(pVertexDataBegin, buffer.data, buffer.totalSize);
            meshPart.m_vertexBuffer->Unmap(0, nullptr);

            meshPart.m_vertexBufferView.BufferLocation = meshPart.m_vertexBuffer->GetGPUVirtualAddress();
            meshPart.m_vertexBufferView.StrideInBytes = buffer.dataStride;
            meshPart.m_vertexBufferView.SizeInBytes = buffer.totalSize;
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
                IID_PPV_ARGS(meshPart.m_normalBuffer.ReleaseAndGetAddressOf())));

            UINT8 * pVertexDataBegin;
            CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
            DX::ThrowIfFailed(meshPart.m_normalBuffer->Map(0, &readRange, reinterpret_cast<void **>(&pVertexDataBegin)));
            std::memcpy(pVertexDataBegin, buffer.data, buffer.totalSize);
            meshPart.m_normalBuffer->Unmap(0, nullptr);

            meshPart.m_normalBufferView.BufferLocation = meshPart.m_normalBuffer->GetGPUVirtualAddress();
            meshPart.m_normalBufferView.StrideInBytes = buffer.dataStride;
            meshPart.m_normalBufferView.SizeInBytes = buffer.totalSize;
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
                IID_PPV_ARGS(meshPart.m_indexBuffer.ReleaseAndGetAddressOf())));

            UINT8 * pVertexDataBegin;
            CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
            DX::ThrowIfFailed(meshPart.m_indexBuffer->Map(0, &readRange, reinterpret_cast<void **>(&pVertexDataBegin)));
            std::memcpy(pVertexDataBegin, buffer.data, buffer.totalSize);
            meshPart.m_indexBuffer->Unmap(0, nullptr);

            // Initialize the index buffer view.
            meshPart.m_indexBufferView.BufferLocation = meshPart.m_indexBuffer->GetGPUVirtualAddress();
            meshPart.m_indexBufferView.Format = GetFormat(buffer.accessor);
            meshPart.m_indexBufferView.SizeInBytes = buffer.totalSize;

            meshPart.m_indexCount = buffer.accessor->count;
        }

        meshPart.m_meshPartColor = HSVtoRBG(std::fmodf(CurrentMeshPartId++ * 0.618033988749895f, 1.0), 0.70f, 0.80f);
    }
}

void D3DMesh::Render(ID3D12GraphicsCommandList * commandList, D3DFrameResource const & currentFrame, DirectX::CXMMATRIX viewProj, std::size_t currentCBIndex)
{
    MeshConstantBuffer meshParameters{};
    meshParameters.worldViewProj = DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&m_worldMatrix) * viewProj);
    meshParameters.world = DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&m_worldMatrix));

    std::size_t meshCBIndex = 0;
    for (auto & meshPart : m_meshParts)
    {
        const std::size_t cbIndex = currentCBIndex + meshCBIndex;

        meshParameters.meshColor = meshPart.m_meshPartColor;
        currentFrame.MeshCB->CopyData(cbIndex, meshParameters);

        D3D12_VERTEX_BUFFER_VIEW * views[2] = { &meshPart.m_vertexBufferView, &meshPart.m_normalBufferView };
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->IASetVertexBuffers(0, 2, views[0]);
        commandList->IASetIndexBuffer(&meshPart.m_indexBufferView);
        commandList->SetGraphicsRootConstantBufferView(1, currentFrame.MeshCB->GetGPUVirtualAddress(cbIndex));
        commandList->DrawIndexedInstanced(meshPart.m_indexCount, 1, 0, 0, 0);

        meshCBIndex++;
    }
}

void D3DMesh::Rotate(float rotationAngleRad)
{
    XMStoreFloat4x4(&m_worldMatrix, DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationY(rotationAngleRad), DirectX::XMMatrixScaling(1, 1, 1)));
}

void D3DMesh::Reset()
{
    for (auto & meshPart : m_meshParts)
    {
        meshPart.m_vertexBuffer.Reset();
        meshPart.m_normalBuffer.Reset();
        meshPart.m_indexBuffer.Reset();
    }
}
