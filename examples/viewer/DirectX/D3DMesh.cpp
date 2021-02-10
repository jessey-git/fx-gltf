// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
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
    fx::gltf::Document const & doc, std::size_t meshIndex, D3DDeviceResources const * deviceResources)
{
    ID3D12Device * device = deviceResources->GetD3DDevice();
    ID3D12GraphicsCommandList * commandList = deviceResources->GetCommandList();

    const D3D12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    m_meshParts.resize(doc.meshes[meshIndex].primitives.size());
    for (std::size_t i = 0; i < doc.meshes[meshIndex].primitives.size(); i++)
    {
        MeshData mesh(doc, meshIndex, i);
        MeshData::BufferInfo const & vBuffer = mesh.VertexBuffer();
        MeshData::BufferInfo const & nBuffer = mesh.NormalBuffer();
        MeshData::BufferInfo const & tBuffer = mesh.TangentBuffer();
        MeshData::BufferInfo const & cBuffer = mesh.TexCoord0Buffer();
        MeshData::BufferInfo const & iBuffer = mesh.IndexBuffer();
        if (!vBuffer.HasData() || !nBuffer.HasData() || !iBuffer.HasData())
        {
            throw std::runtime_error("Only meshes with vertex, normal, and index buffers are supported");
        }

        const std::size_t totalBufferSize =
            static_cast<std::size_t>(vBuffer.TotalSize) +
            static_cast<std::size_t>(nBuffer.TotalSize) +
            static_cast<std::size_t>(tBuffer.TotalSize) +
            static_cast<std::size_t>(cBuffer.TotalSize) +
            static_cast<std::size_t>(iBuffer.TotalSize);

        D3DMeshPart & meshPart = m_meshParts[i];
        const CD3DX12_RESOURCE_DESC resourceDescV = CD3DX12_RESOURCE_DESC::Buffer(totalBufferSize);
        COMUtil::ThrowIfFailed(device->CreateCommittedResource(
            &defaultHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDescV,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(meshPart.DefaultBuffer.ReleaseAndGetAddressOf())));

        COMUtil::ThrowIfFailed(device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDescV,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(meshPart.UploadBuffer.ReleaseAndGetAddressOf())));

        uint8_t * bufferStart{};
        uint32_t offset{};
        const CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
        COMUtil::ThrowIfFailed(meshPart.UploadBuffer->Map(0, &readRange, reinterpret_cast<void **>(&bufferStart)));

        // Copy vertex buffer to upload...
        std::memcpy(bufferStart, vBuffer.Data, vBuffer.TotalSize);
        meshPart.Buffers[SlotVertex].BufferLocation = meshPart.DefaultBuffer->GetGPUVirtualAddress();
        meshPart.Buffers[SlotVertex].StrideInBytes = vBuffer.DataStride;
        meshPart.Buffers[SlotVertex].SizeInBytes = vBuffer.TotalSize;
        offset += vBuffer.TotalSize;

        // Copy normal buffer to upload...
        std::memcpy(bufferStart + offset, nBuffer.Data, nBuffer.TotalSize);
        meshPart.Buffers[SlotNormal].BufferLocation = meshPart.DefaultBuffer->GetGPUVirtualAddress() + offset;
        meshPart.Buffers[SlotNormal].StrideInBytes = nBuffer.DataStride;
        meshPart.Buffers[SlotNormal].SizeInBytes = nBuffer.TotalSize;
        offset += nBuffer.TotalSize;

        if (tBuffer.HasData())
        {
            // Copy tangent buffer to upload...
            std::memcpy(bufferStart + offset, tBuffer.Data, tBuffer.TotalSize);
            meshPart.Buffers[SlotTangent].BufferLocation = meshPart.DefaultBuffer->GetGPUVirtualAddress() + offset;
            meshPart.Buffers[SlotTangent].StrideInBytes = tBuffer.DataStride;
            meshPart.Buffers[SlotTangent].SizeInBytes = tBuffer.TotalSize;
            offset += tBuffer.TotalSize;
        }

        if (cBuffer.HasData())
        {
            // Copy tex-coord buffer to upload...
            std::memcpy(bufferStart + offset, cBuffer.Data, cBuffer.TotalSize);
            meshPart.Buffers[SlotTexCoord0].BufferLocation = meshPart.DefaultBuffer->GetGPUVirtualAddress() + offset;
            meshPart.Buffers[SlotTexCoord0].StrideInBytes = cBuffer.DataStride;
            meshPart.Buffers[SlotTexCoord0].SizeInBytes = cBuffer.TotalSize;
            offset += cBuffer.TotalSize;
        }

        // Copy index buffer to upload...
        std::memcpy(bufferStart + offset, iBuffer.Data, iBuffer.TotalSize);
        meshPart.IndexBufferView.BufferLocation = meshPart.DefaultBuffer->GetGPUVirtualAddress() + offset;
        meshPart.IndexBufferView.Format = Util::GetFormat(iBuffer.Accessor);
        meshPart.IndexBufferView.SizeInBytes = iBuffer.TotalSize;
        meshPart.IndexCount = iBuffer.Accessor->count;

        // Copy from upload to default...
        commandList->CopyBufferRegion(meshPart.DefaultBuffer.Get(), 0, meshPart.UploadBuffer.Get(), 0, totalBufferSize);
        const CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(meshPart.DefaultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_INDEX_BUFFER);
        commandList->ResourceBarrier(1, &barrier);

        Util::BBox boundingBox{};
        boundingBox.Min = DirectX::XMFLOAT3(vBuffer.Accessor->min.data());
        boundingBox.Max = DirectX::XMFLOAT3(vBuffer.Accessor->max.data());
        Util::UnionBBox(m_boundingBox, boundingBox);

        meshPart.UploadBuffer->Unmap(0, nullptr);

        // Set material properties for this mesh piece...
        meshPart.ShaderData.MeshAutoColor = Util::HSVtoRBG(std::fmodf(CurrentMeshPartId++ * 0.618033988749895f, 1.0), 0.65f, 0.65f);
        meshPart.SetMaterial(mesh.Material());
        if (tBuffer.HasData())
        {
            meshPart.ShaderConfig |= ShaderOptions::HAS_TANGENTS;
        }
    }
}

void D3DMesh::SetWorldMatrix(DirectX::XMMATRIX const & baseTransform, DirectX::XMFLOAT3 const & centerTranslation, float rotationY, float scalingFactor)
{
    const DirectX::XMMATRIX translation = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&centerTranslation));
    const DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationY(rotationY);
    const DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(scalingFactor, scalingFactor, scalingFactor);
    DirectX::XMStoreFloat4x4(&m_worldMatrix, baseTransform * translation * rotation * scale);
}

void D3DMesh::Render(D3DRenderContext & renderContext)
{
    MeshConstantBuffer meshParameters{};
    meshParameters.WorldViewProj = DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&m_worldMatrix) * renderContext.ViewProj);
    meshParameters.World = DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&m_worldMatrix));

    std::size_t meshCBIndex = 0;
    for (auto & meshPart : m_meshParts)
    {
        // Ensure we can correctly draw this part of the mesh. An optimization would be to reduce the number of calls to SetPipelineState further by sorting the meshes...
        const ShaderOptions options = renderContext.OverrideShaderOptions == ShaderOptions::None ? meshPart.ShaderConfig : renderContext.OverrideShaderOptions;
        if (options != renderContext.CurrentShaderOptions)
        {
            renderContext.CommandList->SetPipelineState(renderContext.PipelineStateMap[options].Get());
            renderContext.CurrentShaderOptions = options;
        }

        const std::size_t cbIndex = renderContext.CurrentCBIndex + meshCBIndex;
        meshParameters.MaterialIndex = static_cast<int>(cbIndex);
        renderContext.CurrentFrame.MeshCB->CopyData(cbIndex, meshParameters);
        renderContext.CurrentFrame.MeshDataBuffer->CopyData(cbIndex, meshPart.ShaderData);

        renderContext.CommandList->IASetVertexBuffers(0, 4, &meshPart.Buffers[0]);
        renderContext.CommandList->IASetIndexBuffer(&meshPart.IndexBufferView);
        renderContext.CommandList->SetGraphicsRootConstantBufferView(1, renderContext.CurrentFrame.MeshCB->GetGPUVirtualAddress(cbIndex));
        renderContext.CommandList->DrawIndexedInstanced(meshPart.IndexCount, 1, 0, 0, 0);

        meshCBIndex++;
    }

    renderContext.CurrentCBIndex += m_meshParts.size();
}

void D3DMesh::FinishUpload()
{
    for (auto & meshPart : m_meshParts)
    {
        meshPart.UploadBuffer.Reset();
    }
}

void D3DMesh::Reset()
{
    for (auto & meshPart : m_meshParts)
    {
        meshPart.DefaultBuffer.Reset();
    }
}

void D3DMesh::D3DMeshPart::SetMaterial(MaterialData const & materialData)
{
    if (materialData.HasData())
    {
        auto material = materialData.Data();
        ShaderData.BaseColorIndex = material.pbrMetallicRoughness.baseColorTexture.index;
        ShaderData.BaseColorFactor = DirectX::XMFLOAT4(material.pbrMetallicRoughness.baseColorFactor.data());

        ShaderData.MetalRoughIndex = material.pbrMetallicRoughness.metallicRoughnessTexture.index;
        ShaderData.MetallicFactor = material.pbrMetallicRoughness.metallicFactor;
        ShaderData.RoughnessFactor = material.pbrMetallicRoughness.roughnessFactor;

        ShaderData.NormalIndex = material.normalTexture.index;
        ShaderData.NormalScale = material.normalTexture.scale;

        ShaderData.AOIndex = material.occlusionTexture.index;
        ShaderData.AOStrength = material.occlusionTexture.strength;

        ShaderData.EmissiveIndex = material.emissiveTexture.index;
        ShaderData.EmissiveFactor = DirectX::XMFLOAT3(material.emissiveFactor.data());

        ShaderOptions options = ShaderOptions::None;
        if (ShaderData.BaseColorIndex >= 0)
            options |= ShaderOptions::HAS_BASECOLORMAP;
        if (ShaderData.NormalIndex >= 0)
            options |= ShaderOptions::HAS_NORMALMAP;
        if (ShaderData.MetalRoughIndex >= 0)
            options |= ShaderOptions::HAS_METALROUGHNESSMAP;
        if (ShaderData.AOIndex >= 0)
        {
            options |= ShaderOptions::HAS_OCCLUSIONMAP;

            if (ShaderData.AOIndex == ShaderData.MetalRoughIndex)
            {
                options |= ShaderOptions::HAS_OCCLUSIONMAP_COMBINED;
            }
        }
        if (ShaderData.EmissiveIndex >= 0)
            options |= ShaderOptions::HAS_EMISSIVEMAP;

        if (options == ShaderOptions::None)
        {
            options = ShaderOptions::USE_FACTORS_ONLY;
        }

        ShaderConfig = options;
    }
    else
    {
        // Use a default material
        ShaderConfig = ShaderOptions::USE_FACTORS_ONLY;
        ShaderData.BaseColorFactor = DirectX::XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
        ShaderData.MetallicFactor = 0;
        ShaderData.RoughnessFactor = 0.5f;
    }
}
