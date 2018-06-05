// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <d3d12.h>
#include <libpng16/png.h>
#include <string>
#include <vector>
#include <wrl.h>

#include "D3DDeviceResources.h"

class D3DTextureSet
{
public:
    void Initialize(std::vector<std::string> const & textures);

    void LoadToMemory(
        DX::D3DDeviceResources const * deviceResources,
        Microsoft::WRL::ComPtr<ID3D12Resource> & defaultBuffer,
        Microsoft::WRL::ComPtr<ID3D12Resource> & uploadBuffer,
        UINT16 depthOrArraySize,
        UINT16 mipChainLength);

private:
    std::vector<png_image> m_images{};

    std::size_t m_totalSize{};
};
