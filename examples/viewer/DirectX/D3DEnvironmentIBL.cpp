// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#include "stdafx.h"
#include <vector>

#include "D3DEnvironmentIBL.h"
#include "D3DTextureSet.h"
#include "D3DUtil.h"
#include "ImageData.h"
#include "StringFormatter.h"

void D3DEnvironmentIBL::Create(D3DDeviceResources const * deviceResources)
{
    D3DTextureSet diffuseSet;
    D3DTextureSet specularSet;
    D3DTextureSet lutSet;

    std::vector<ImageData> diffuseTextures;
    std::vector<ImageData> specularTextures;

    for (auto axis : { "x", "y", "z" })
    {
        for (auto dir : { "pos", "neg" })
        {
            diffuseTextures.emplace_back(
                fx::common::StringFormatter::Format("Assets/Environment/diffuse_{0}{1}.png", dir, axis));

            for (auto mipIndex : { 0, 1, 2, 3, 4, 5, 6 })
            {
                specularTextures.emplace_back(
                    fx::common::StringFormatter::Format("Assets/Environment/specular_{0}{1}_{2}.png", dir, axis, mipIndex));
            }
        }
    }

    diffuseSet.Initialize(diffuseTextures);
    specularSet.Initialize(specularTextures);
    lutSet.Initialize({ ImageData("Assets/Environment/LUT.png") });

    diffuseSet.LoadToMemory(deviceResources, m_data.DiffuseBuffer, m_data.DiffuseUploadBuffer, 6, 1);
    specularSet.LoadToMemory(deviceResources, m_data.SpecularBuffer, m_data.SpecularUploadBuffer, 6, 7);
    lutSet.LoadToMemory(deviceResources, m_data.LUTBuffer, m_data.LUTUploadBuffer, 1, 1);
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
    descriptor.Offset(1, size);
}

void D3DEnvironmentIBL::FinishUpload()
{
    m_data.DiffuseUploadBuffer.Reset();
    m_data.SpecularUploadBuffer.Reset();
    m_data.LUTUploadBuffer.Reset();
}
