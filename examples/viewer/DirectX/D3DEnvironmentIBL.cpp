// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#include "stdafx.h"
#include <libpng16/png.h>
#include <vector>

#include "D3DEnvironmentIBL.h"
#include "D3DUtil.h"

void D3DEnvironmentIBL::Create(DX::D3DDeviceResources const * deviceResources)
{
    ID3D12Device * device = deviceResources->GetD3DDevice();
    ID3D12GraphicsCommandList * commandList = deviceResources->GetCommandList();

    std::vector<png_image> faces(6);
    for (auto & face : faces)
    {
        face.version = PNG_IMAGE_VERSION;
    }

    if (png_image_begin_read_from_file(&faces[0], "Assets/Environment/diffuse_posx.png") != 0 &&
        png_image_begin_read_from_file(&faces[1], "Assets/Environment/diffuse_negx.png") != 0 &&
        png_image_begin_read_from_file(&faces[2], "Assets/Environment/diffuse_posy.png") != 0 &&
        png_image_begin_read_from_file(&faces[3], "Assets/Environment/diffuse_negy.png") != 0 &&
        png_image_begin_read_from_file(&faces[4], "Assets/Environment/diffuse_posz.png") != 0 &&
        png_image_begin_read_from_file(&faces[5], "Assets/Environment/diffuse_negz.png") != 0)
    {
        std::size_t totalSize = 0;
        for (auto & face : faces)
        {
            face.format = PNG_FORMAT_RGBA;
            totalSize += PNG_IMAGE_SIZE(face);
        }

        const D3D12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

        D3D12_RESOURCE_DESC desc = {};
        desc.Width = static_cast<UINT>(faces[0].width);
        desc.Height = static_cast<UINT>(faces[0].height);
        desc.MipLevels = 1;
        desc.DepthOrArraySize = 6;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        const CD3DX12_RESOURCE_DESC resourceDescV = CD3DX12_RESOURCE_DESC::Buffer(totalSize);
        DX::ThrowIfFailed(device->CreateCommittedResource(
            &defaultHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(m_data.DiffuseBuffer.ReleaseAndGetAddressOf())));

        DX::ThrowIfFailed(device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDescV,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(m_data.DiffuseUploadBuffer.ReleaseAndGetAddressOf())));

        uint8_t * bufferStart{};
        const CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
        DX::ThrowIfFailed(m_data.DiffuseUploadBuffer->Map(0, &readRange, reinterpret_cast<void **>(&bufferStart)));

        std::size_t offset = 0;
        std::vector<D3D12_SUBRESOURCE_DATA> subresources(6);
        for (std::size_t i = 0; i < 6; i++)
        {
            png_image & face = faces[i];

            // Load the PNG data right into the upload buffer...
            const int result = png_image_finish_read(&face, nullptr, bufferStart + offset, 0, nullptr);
            if (result != 1)
            {
                throw std::runtime_error(face.message);
            }

            subresources[i].pData = bufferStart + offset;
            subresources[i].RowPitch = face.width * 4;
            subresources[i].SlicePitch = subresources[i].RowPitch * face.height;

            offset += PNG_IMAGE_SIZE(face);
        }

        UpdateSubresources<6>(commandList, m_data.DiffuseBuffer.Get(), m_data.DiffuseUploadBuffer.Get(), 0, 0, 6, &subresources[0]);
        const CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_data.DiffuseBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        commandList->ResourceBarrier(1, &barrier);
    }

    if (png_image_begin_read_from_file(&faces[0], "Assets/Environment/specular_posx_0.png") != 0 &&
        png_image_begin_read_from_file(&faces[1], "Assets/Environment/specular_negx_0.png") != 0 &&
        png_image_begin_read_from_file(&faces[2], "Assets/Environment/specular_posy_0.png") != 0 &&
        png_image_begin_read_from_file(&faces[3], "Assets/Environment/specular_negy_0.png") != 0 &&
        png_image_begin_read_from_file(&faces[4], "Assets/Environment/specular_posz_0.png") != 0 &&
        png_image_begin_read_from_file(&faces[5], "Assets/Environment/specular_negz_0.png") != 0)
    {
        std::size_t totalSize = 0;
        for (auto & face : faces)
        {
            face.format = PNG_FORMAT_RGBA;
            totalSize += PNG_IMAGE_SIZE(face);
        }

        const D3D12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

        D3D12_RESOURCE_DESC desc = {};
        desc.Width = static_cast<UINT>(faces[0].width);
        desc.Height = static_cast<UINT>(faces[0].height);
        desc.MipLevels = 1;
        desc.DepthOrArraySize = 6;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        const CD3DX12_RESOURCE_DESC resourceDescV = CD3DX12_RESOURCE_DESC::Buffer(totalSize);
        DX::ThrowIfFailed(device->CreateCommittedResource(
            &defaultHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(m_data.SpecularBuffer.ReleaseAndGetAddressOf())));

        DX::ThrowIfFailed(device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDescV,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(m_data.SpecularUploadBuffer.ReleaseAndGetAddressOf())));

        uint8_t * bufferStart{};
        const CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
        DX::ThrowIfFailed(m_data.SpecularUploadBuffer->Map(0, &readRange, reinterpret_cast<void **>(&bufferStart)));

        std::size_t offset = 0;
        std::vector<D3D12_SUBRESOURCE_DATA> subresources(6);
        for (std::size_t i = 0; i < 6; i++)
        {
            png_image & face = faces[i];

            // Load the PNG data right into the upload buffer...
            const int result = png_image_finish_read(&face, nullptr, bufferStart + offset, 0, nullptr);
            if (result != 1)
            {
                throw std::runtime_error(face.message);
            }

            subresources[i].pData = bufferStart + offset;
            subresources[i].RowPitch = face.width * 4;
            subresources[i].SlicePitch = subresources[i].RowPitch * face.height;

            offset += PNG_IMAGE_SIZE(face);
        }

        UpdateSubresources<6>(commandList, m_data.SpecularBuffer.Get(), m_data.SpecularUploadBuffer.Get(), 0, 0, 6, &subresources[0]);
        const CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_data.SpecularBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        commandList->ResourceBarrier(1, &barrier);
    }

    png_image lut{};
    lut.version = PNG_IMAGE_VERSION;
    if (png_image_begin_read_from_file(&lut, "Assets/Environment/LUT.png") != 0)
    {
        lut.format = PNG_FORMAT_RGBA;
        const std::size_t totalSize = PNG_IMAGE_SIZE(lut);

        const D3D12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

        D3D12_RESOURCE_DESC desc = {};
        desc.Width = static_cast<UINT>(lut.width);
        desc.Height = static_cast<UINT>(lut.height);
        desc.MipLevels = 1;
        desc.DepthOrArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        const CD3DX12_RESOURCE_DESC resourceDescV = CD3DX12_RESOURCE_DESC::Buffer(totalSize);
        DX::ThrowIfFailed(device->CreateCommittedResource(
            &defaultHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(m_data.LUTBuffer.ReleaseAndGetAddressOf())));

        DX::ThrowIfFailed(device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDescV,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(m_data.LUTUploadBuffer.ReleaseAndGetAddressOf())));

        uint8_t * bufferStart{};
        const CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
        DX::ThrowIfFailed(m_data.LUTUploadBuffer->Map(0, &readRange, reinterpret_cast<void **>(&bufferStart)));

        D3D12_SUBRESOURCE_DATA subresource;
        const int result = png_image_finish_read(&lut, nullptr, bufferStart, 0, nullptr);
        if (result != 1)
        {
            throw std::runtime_error(lut.message);
        }

        subresource.pData = bufferStart;
        subresource.RowPitch = lut.width * 4;
        subresource.SlicePitch = subresource.RowPitch * lut.height;

        UpdateSubresources<1>(commandList, m_data.LUTBuffer.Get(), m_data.LUTUploadBuffer.Get(), 0, 0, 1, &subresource);
        const CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_data.LUTBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        commandList->ResourceBarrier(1, &barrier);
    }
}

void D3DEnvironmentIBL::CreateSRV(ID3D12Device * device, CD3DX12_CPU_DESCRIPTOR_HANDLE & descriptor)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC diffuseDesc = {};
    diffuseDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    diffuseDesc.Format = m_data.DiffuseBuffer->GetDesc().Format;
    diffuseDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    diffuseDesc.Texture2D.MostDetailedMip = 0;
    diffuseDesc.Texture2D.MipLevels = m_data.DiffuseBuffer->GetDesc().MipLevels;
    diffuseDesc.Texture2D.ResourceMinLODClamp = 0.0f;

    D3D12_SHADER_RESOURCE_VIEW_DESC specularDesc = {};
    specularDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    specularDesc.Format = m_data.SpecularBuffer->GetDesc().Format;
    specularDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    specularDesc.Texture2D.MostDetailedMip = 0;
    specularDesc.Texture2D.MipLevels = m_data.SpecularBuffer->GetDesc().MipLevels;
    specularDesc.Texture2D.ResourceMinLODClamp = 0.0f;

    D3D12_SHADER_RESOURCE_VIEW_DESC lutDesc = {};
    lutDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    lutDesc.Format = m_data.LUTBuffer->GetDesc().Format;
    lutDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    lutDesc.Texture2D.MostDetailedMip = 0;
    lutDesc.Texture2D.MipLevels = m_data.LUTBuffer->GetDesc().MipLevels;
    lutDesc.Texture2D.ResourceMinLODClamp = 0.0f;

    const uint32_t size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    device->CreateShaderResourceView(m_data.DiffuseBuffer.Get(), &diffuseDesc, descriptor);
    descriptor.Offset(1, size);
    device->CreateShaderResourceView(m_data.SpecularBuffer.Get(), &specularDesc, descriptor);
    descriptor.Offset(1, size);
    device->CreateShaderResourceView(m_data.LUTBuffer.Get(), &lutDesc, descriptor);
}

void D3DEnvironmentIBL::FinishUpload()
{
    m_data.DiffuseUploadBuffer.Reset();
    m_data.SpecularUploadBuffer.Reset();
    m_data.LUTUploadBuffer.Reset();
}