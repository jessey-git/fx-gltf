// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#include "stdafx.h"

#include "D3DTextureSet.h"
#include "D3DUtil.h"

void D3DTextureSet::Initialize(std::vector<std::string> const & textures)
{
    m_images.resize(textures.size());

    for (std::size_t i = 0; i < textures.size(); i++)
    {
        png_image & image = m_images[i];
        image.version = PNG_IMAGE_VERSION;

        if (png_image_begin_read_from_file(&image, textures[i].c_str()) != 0)
        {
            image.format = PNG_FORMAT_RGBA;

            m_totalSize += Util::ResourceSize(PNG_IMAGE_SIZE(image));
        }
        else
        {
            throw std::runtime_error(image.message);
        }
    }
}

void D3DTextureSet::LoadToMemory(
    DX::D3DDeviceResources const * deviceResources,
    Microsoft::WRL::ComPtr<ID3D12Resource> & defaultBuffer,
    Microsoft::WRL::ComPtr<ID3D12Resource> & uploadBuffer,
    UINT16 depthOrArraySize,
    UINT16 mipChainLength)
{
    ID3D12Device * device = deviceResources->GetD3DDevice();
    ID3D12GraphicsCommandList * commandList = deviceResources->GetCommandList();

    const D3D12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    D3D12_RESOURCE_DESC texDesc{};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Alignment = 0;
    texDesc.Width = m_images[0].width;
    texDesc.Height = m_images[0].height;
    texDesc.DepthOrArraySize = depthOrArraySize;
    texDesc.MipLevels = mipChainLength;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    const CD3DX12_RESOURCE_DESC resourceDescV = CD3DX12_RESOURCE_DESC::Buffer(m_totalSize);
    DX::ThrowIfFailed(device->CreateCommittedResource(
        &defaultHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(defaultBuffer.ReleaseAndGetAddressOf())));

    DX::ThrowIfFailed(device->CreateCommittedResource(
        &uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDescV,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(uploadBuffer.ReleaseAndGetAddressOf())));

    uint8_t * bufferStart{};
    const CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
    DX::ThrowIfFailed(uploadBuffer->Map(0, &readRange, reinterpret_cast<void **>(&bufferStart)));

    const UINT totalSubresources = depthOrArraySize * mipChainLength;

    std::size_t offset = 0;
    std::vector<D3D12_SUBRESOURCE_DATA> subresources(totalSubresources);
    for (std::size_t i = 0; i < totalSubresources; i++)
    {
        png_image & image = m_images[i];

        // Load the PNG data right into the upload buffer...
        const int result = png_image_finish_read(&image, nullptr, bufferStart + offset, 0, nullptr);
        if (result != 1)
        {
            throw std::runtime_error(image.message);
        }

        subresources[i].pData = bufferStart + offset;
        subresources[i].RowPitch = static_cast<int64_t>(image.width) * 4;
        subresources[i].SlicePitch = subresources[i].RowPitch * image.height;

        offset += Util::ResourceSize(PNG_IMAGE_SIZE(image));
    }

    UpdateSubresources(commandList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, totalSubresources, &subresources[0]);
    const CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList->ResourceBarrier(1, &barrier);
}
