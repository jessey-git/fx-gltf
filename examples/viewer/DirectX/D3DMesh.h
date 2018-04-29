// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <fx/gltf.h>
#include <vector>

#include "D3DDeviceResources.h"

class D3DMesh
{
public:
    void CreateDeviceDependentResources(
        fx::gltf::Document const & doc, std::size_t meshIndex, std::unique_ptr<DX::D3DDeviceResources> const & deviceResources);

    void Render(ID3D12GraphicsCommandList * commandList);

    void Rotate(float rotationAngleRad);

    void Reset();

    DirectX::XMFLOAT4X4 & WorldMatrix() { return m_worldMatrix; }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_normalBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;

    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView{};
    D3D12_VERTEX_BUFFER_VIEW m_normalBufferView{};
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView{};

    DirectX::XMFLOAT4X4 m_worldMatrix{};

    uint32_t m_indexCount{};
};
