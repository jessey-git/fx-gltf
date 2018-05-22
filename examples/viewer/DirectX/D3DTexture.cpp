// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#include "stdafx.h"
#include <fx/gltf.h>
#include <libpng16/png.h>
#include <vector>

#include "D3DTexture.h"

void D3DTexture::Create(std::string const & texture, DX::D3DDeviceResources const * deviceResources)
{
    png_image image{};
    image.version = PNG_IMAGE_VERSION;

    if (png_image_begin_read_from_file(&image, texture.c_str()) != 0)
    {
        image.format = PNG_FORMAT_RGBA;

        ID3D12Device * device = deviceResources->GetD3DDevice();
        ID3D12GraphicsCommandList * commandList = deviceResources->GetCommandList();

        const D3D12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        const std::size_t size = PNG_IMAGE_SIZE(image);

        D3D12_RESOURCE_DESC texDesc{};
        texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        texDesc.Alignment = 0;
        texDesc.Width = image.width;
        texDesc.Height = (uint32_t)image.height;
        texDesc.DepthOrArraySize = 1;
        texDesc.MipLevels = 1;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        const CD3DX12_RESOURCE_DESC resourceDescV = CD3DX12_RESOURCE_DESC::Buffer(size);
        DX::ThrowIfFailed(device->CreateCommittedResource(
            &defaultHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &texDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(m_data.m_mainBuffer.ReleaseAndGetAddressOf())));

        DX::ThrowIfFailed(device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDescV,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(m_data.m_uploadBuffer.ReleaseAndGetAddressOf())));

        uint8_t * bufferStart{};
        const CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
        DX::ThrowIfFailed(m_data.m_uploadBuffer->Map(0, &readRange, reinterpret_cast<void **>(&bufferStart)));

        // Load the PNG data right into the upload buffer...
        const int result = png_image_finish_read(&image, nullptr, bufferStart, 0, nullptr);
        if (result != 1)
        {
            throw std::runtime_error(image.message);
        }

        // Copy from upload to default...
        D3D12_SUBRESOURCE_DATA texResource;
        texResource.pData = bufferStart;
        texResource.RowPitch = image.width * 4;
        texResource.SlicePitch = texResource.RowPitch * image.height;
        UpdateSubresources(commandList, m_data.m_mainBuffer.Get(), m_data.m_uploadBuffer.Get(), 0, 0, 1, &texResource);

        const CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_data.m_mainBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        commandList->ResourceBarrier(1, &barrier);
    }
}

void D3DTexture::CreateSRV(ID3D12Device * device, CD3DX12_CPU_DESCRIPTOR_HANDLE const & descriptor)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = m_data.m_mainBuffer->GetDesc().Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = m_data.m_mainBuffer->GetDesc().MipLevels;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

    device->CreateShaderResourceView(m_data.m_mainBuffer.Get(), &srvDesc, descriptor);
}

void D3DTexture::FinishUpload()
{
    m_data.m_uploadBuffer.Reset();
}