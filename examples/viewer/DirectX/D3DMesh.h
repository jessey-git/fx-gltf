// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <fx/gltf.h>
#include <vector>

#include "D3DDeviceResources.h"
#include "D3DUtil.h"

class D3DMesh
{
public:
    void CreateDeviceDependentResources(
        fx::gltf::Document const & doc, std::size_t meshIndex, std::unique_ptr<DX::D3DDeviceResources> const & deviceResources);

    void SetWorldMatrix(DirectX::XMMATRIX m, float scalingFactor)
    {
        XMStoreFloat4x4(&m_worldMatrix, DirectX::XMMatrixMultiply(m, DirectX::XMMatrixScaling(scalingFactor, scalingFactor, scalingFactor)));
    }

    void Render(ID3D12GraphicsCommandList * commandList, D3DFrameResource const & currentFrame, DirectX::CXMMATRIX viewProj, std::size_t currentCBIndex);

    void Reset();

    Util::BBox MeshBBox() noexcept { return m_boundingBox; }

    std::size_t MeshPartCount() noexcept { return m_meshParts.size(); }

private:
    struct D3DMeshPart
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer{};
        Microsoft::WRL::ComPtr<ID3D12Resource> m_normalBuffer{};
        Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer{};

        D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView{};
        D3D12_VERTEX_BUFFER_VIEW m_normalBufferView{};
        D3D12_INDEX_BUFFER_VIEW m_indexBufferView{};

        uint32_t m_indexCount{};

        DirectX::XMFLOAT3 m_meshPartColor{};
    };

    DirectX::XMFLOAT4X4 m_worldMatrix{};

    std::vector<D3DMeshPart> m_meshParts{};

    Util::BBox m_boundingBox;

    static uint32_t CurrentMeshPartId;
};
