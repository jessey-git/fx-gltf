// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <d3d12.h>
#include <string>
#include <vector>
#include <wincodec.h>
#include <wrl.h>

#include "D3DDeviceResources.h"
#include "ImageData.h"

class D3DTextureSet
{
public:
    void Initialize(std::vector<ImageData> const & textures);

    void LoadToMemory(
        D3DDeviceResources const * deviceResources,
        Microsoft::WRL::ComPtr<ID3D12Resource> & defaultBuffer,
        Microsoft::WRL::ComPtr<ID3D12Resource> & uploadBuffer,
        UINT16 depthOrArraySize,
        UINT16 mipChainLength);

private:
    struct Image
    {
        Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame{};
        Microsoft::WRL::ComPtr<IWICFormatConverter> formatConverter{};

        uint32_t width{};
        uint32_t height{};
        uint32_t size{};
    };

    std::vector<Image> m_images{};

    std::size_t m_totalSize{};
};
