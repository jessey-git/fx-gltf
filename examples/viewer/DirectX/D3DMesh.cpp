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
    fx::gltf::Document const & doc, std::size_t meshIndex, DX::D3DDeviceResources * deviceResources)
{
    ID3D12Device * device = deviceResources->GetD3DDevice();
    ID3D12GraphicsCommandList * commandList = deviceResources->GetCommandList();

    const D3D12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
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
        const MeshData::BufferInfo vBuffer = mesh.VertexBuffer();
        const MeshData::BufferInfo nBuffer = mesh.NormalBuffer();
        const MeshData::BufferInfo iBuffer = mesh.IndexBuffer();
        const std::size_t totalBufferSize =
            static_cast<std::size_t>(vBuffer.totalSize) +
            static_cast<std::size_t>(nBuffer.totalSize) +
            static_cast<std::size_t>(iBuffer.totalSize);

        const CD3DX12_RESOURCE_DESC resourceDescV = CD3DX12_RESOURCE_DESC::Buffer(totalBufferSize);
        DX::ThrowIfFailed(device->CreateCommittedResource(
            &defaultHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDescV,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(meshPart.m_mainBuffer.ReleaseAndGetAddressOf())));

        DX::ThrowIfFailed(device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDescV,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(meshPart.m_uploadBuffer.ReleaseAndGetAddressOf())));

        uint8_t * bufferStart{};
        uint32_t offset{};
        const CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
        DX::ThrowIfFailed(meshPart.m_uploadBuffer->Map(0, &readRange, reinterpret_cast<void **>(&bufferStart)));

        // Copy vertex buffer to upload...
        std::memcpy(bufferStart, vBuffer.data, vBuffer.totalSize);
        meshPart.m_vertexBufferView.BufferLocation = meshPart.m_mainBuffer->GetGPUVirtualAddress();
        meshPart.m_vertexBufferView.StrideInBytes = vBuffer.dataStride;
        meshPart.m_vertexBufferView.SizeInBytes = vBuffer.totalSize;
        offset += vBuffer.totalSize;

        // Copy normal buffer to upload...
        std::memcpy(bufferStart + offset, nBuffer.data, nBuffer.totalSize);
        meshPart.m_normalBufferView.BufferLocation = meshPart.m_mainBuffer->GetGPUVirtualAddress() + offset;
        meshPart.m_normalBufferView.StrideInBytes = nBuffer.dataStride;
        meshPart.m_normalBufferView.SizeInBytes = nBuffer.totalSize;
        offset += nBuffer.totalSize;

        // Copy index buffer to upload...
        std::memcpy(bufferStart + offset, iBuffer.data, iBuffer.totalSize);
        meshPart.m_indexBufferView.BufferLocation = meshPart.m_mainBuffer->GetGPUVirtualAddress() + offset;
        meshPart.m_indexBufferView.Format = Util::GetFormat(iBuffer.accessor);
        meshPart.m_indexBufferView.SizeInBytes = iBuffer.totalSize;
        meshPart.m_indexCount = iBuffer.accessor->count;

        // Copy from upload to default...
        commandList->CopyBufferRegion(
            meshPart.m_mainBuffer.Get(), 0, meshPart.m_uploadBuffer.Get(), 0, totalBufferSize);
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(meshPart.m_mainBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_INDEX_BUFFER);
        commandList->ResourceBarrier(1, &barrier);

        Util::BBox boundingBox{};
        boundingBox.min = DirectX::XMFLOAT3(vBuffer.accessor->min.data());
        boundingBox.max = DirectX::XMFLOAT3(vBuffer.accessor->max.data());
        Util::AdjustBBox(m_boundingBox, boundingBox);

        meshPart.m_uploadBuffer->Unmap(0, nullptr);
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

void D3DMesh::FinishUpload()
{
    for (auto & meshPart : m_meshParts)
    {
        meshPart.m_uploadBuffer.Reset();
    }
}

void D3DMesh::Reset()
{
    for (auto & meshPart : m_meshParts)
    {
        meshPart.m_mainBuffer.Reset();
    }
}
