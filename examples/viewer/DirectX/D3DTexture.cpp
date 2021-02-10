// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#include "stdafx.h"

#include "D3DTexture.h"
#include "D3DTextureSet.h"
#include "D3DUtil.h"

void D3DTexture::Create(ImageData const & texture, D3DDeviceResources const * deviceResources)
{
    D3DTextureSet texSet;

    texSet.Initialize({ texture });
    texSet.LoadToMemory(deviceResources, m_data.DefaultBuffer, m_data.UploadBuffer, 1, 1);
}

void D3DTexture::CreateSRV(ID3D12Device * device, CD3DX12_CPU_DESCRIPTOR_HANDLE const & descriptor)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = m_data.DefaultBuffer->GetDesc().Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = m_data.DefaultBuffer->GetDesc().MipLevels;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

    device->CreateShaderResourceView(m_data.DefaultBuffer.Get(), &srvDesc, descriptor);
}

void D3DTexture::FinishUpload()
{
    m_data.UploadBuffer.Reset();
}
