// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#include "stdafx.h"
#include <fx/gltf.h>
#include <vector>

#include "D3DMesh.h"
#include "MeshData.h"

uint32_t D3DMesh::CurrentMeshPartId = 1;

void D3DMesh::Create(
    fx::gltf::Document const & doc, std::size_t meshIndex, ID3D12Device * device)
{
    const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    m_meshParts.resize(doc.meshes[meshIndex].primitives.size());
    for (std::size_t i = 0; i < doc.meshes[meshIndex].primitives.size(); i++)
    {
        MeshData mesh(doc, meshIndex, i);
        if (!mesh.HasVertexData() || !mesh.HasNormalData() || !mesh.HasIndexData())
        {
            throw std::runtime_error("Only meshes with vertex, normal, and index buffers are supported");
        }

        D3DMeshPart & meshPart = m_meshParts[i];

        // Create the vertex buffer
        {
            const MeshData::BufferInfo buffer = mesh.VertexBuffer();
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
            const CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
            DX::ThrowIfFailed(meshPart.m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void **>(&pVertexDataBegin)));
            std::memcpy(pVertexDataBegin, buffer.data, buffer.totalSize);
            meshPart.m_vertexBuffer->Unmap(0, nullptr);

            meshPart.m_vertexBufferView.BufferLocation = meshPart.m_vertexBuffer->GetGPUVirtualAddress();
            meshPart.m_vertexBufferView.StrideInBytes = buffer.dataStride;
            meshPart.m_vertexBufferView.SizeInBytes = buffer.totalSize;

            Util::BBox boundingBox{};
            boundingBox.min = DirectX::XMFLOAT3(buffer.accessor->min.data());
            boundingBox.max = DirectX::XMFLOAT3(buffer.accessor->max.data());
            Util::AdjustBBox(m_boundingBox, boundingBox);
        }

        // Create the normal buffer
        {
            const MeshData::BufferInfo buffer = mesh.NormalBuffer();
            const CD3DX12_RESOURCE_DESC resourceDescN = CD3DX12_RESOURCE_DESC::Buffer(buffer.totalSize);
            DX::ThrowIfFailed(device->CreateCommittedResource(
                &uploadHeapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDescN,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(meshPart.m_normalBuffer.ReleaseAndGetAddressOf())));

            UINT8 * pVertexDataBegin;
            const CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
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
            const MeshData::BufferInfo buffer = mesh.IndexBuffer();
            const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(buffer.totalSize);
            DX::ThrowIfFailed(device->CreateCommittedResource(
                &uploadHeapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(meshPart.m_indexBuffer.ReleaseAndGetAddressOf())));

            UINT8 * pVertexDataBegin;
            const CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
            DX::ThrowIfFailed(meshPart.m_indexBuffer->Map(0, &readRange, reinterpret_cast<void **>(&pVertexDataBegin)));
            std::memcpy(pVertexDataBegin, buffer.data, buffer.totalSize);
            meshPart.m_indexBuffer->Unmap(0, nullptr);

            // Initialize the index buffer view.
            meshPart.m_indexBufferView.BufferLocation = meshPart.m_indexBuffer->GetGPUVirtualAddress();
            meshPart.m_indexBufferView.Format = Util::GetFormat(buffer.accessor);
            meshPart.m_indexBufferView.SizeInBytes = buffer.totalSize;

            meshPart.m_indexCount = buffer.accessor->count;
        }

        meshPart.m_meshPartColor = Util::HSVtoRBG(std::fmodf(CurrentMeshPartId++ * 0.618033988749895f, 1.0), 0.65f, 0.65f);
    }
}

void D3DMesh::SetWorldMatrix(DirectX::XMMATRIX const & baseTransform, DirectX::XMFLOAT3 const & centerTranslation, float rotationY, float scalingFactor)
{
    using namespace DirectX;
    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&centerTranslation));
    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationY(rotationY);
    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(scalingFactor, scalingFactor, scalingFactor);
    DirectX::XMStoreFloat4x4(&m_worldMatrix, baseTransform * translation * rotation * scale);
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

        D3D12_VERTEX_BUFFER_VIEW const * views[2] = { &meshPart.m_vertexBufferView, &meshPart.m_normalBufferView };
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->IASetVertexBuffers(0, 2, views[0]);
        commandList->IASetIndexBuffer(&meshPart.m_indexBufferView);
        commandList->SetGraphicsRootConstantBufferView(1, currentFrame.MeshCB->GetGPUVirtualAddress(cbIndex));
        commandList->DrawIndexedInstanced(meshPart.m_indexCount, 1, 0, 0, 0);

        meshCBIndex++;
    }
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
