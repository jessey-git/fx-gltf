// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <array>
#include <fx/gltf.h>
#include <vector>

#include "D3DDeviceResources.h"
#include "D3DRenderContext.h"
#include "D3DUtil.h"
#include "MaterialData.h"
#include "ShaderOptions.h"

class D3DMesh
{
public:
    void Create(
        fx::gltf::Document const & doc, std::size_t meshIndex, D3DDeviceResources const * deviceResources);

    void SetWorldMatrix(DirectX::XMMATRIX const & baseTransform, DirectX::XMFLOAT3 const & centerTranslation, float rotationY, float scalingFactor);

    void Render(D3DRenderContext & renderContext);

    void FinishUpload();

    void Reset();

    Util::BBox const & MeshBBox() const noexcept
    {
        return m_boundingBox;
    }

    std::size_t MeshPartCount() const noexcept
    {
        return m_meshParts.size();
    }

    std::vector<ShaderOptions> GetRequiredShaderOptions() const
    {
        std::vector<ShaderOptions> requiredShaderOptions{};
        for (auto const & meshPart : m_meshParts)
        {
            requiredShaderOptions.push_back(meshPart.ShaderConfig);
        }

        return requiredShaderOptions;
    }

    static const UINT SlotVertex = 0;
    static const UINT SlotNormal = 1;
    static const UINT SlotTangent = 2;
    static const UINT SlotTexCoord0 = 3;

private:
    struct D3DMeshPart
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> DefaultBuffer{};
        Microsoft::WRL::ComPtr<ID3D12Resource> UploadBuffer{};

        // Views 0-3 should match the input-slot order defined by the PSO
        std::array<D3D12_VERTEX_BUFFER_VIEW, 4> Buffers{};

        D3D12_INDEX_BUFFER_VIEW IndexBufferView{};

        uint32_t IndexCount{};

        MeshShaderData ShaderData{};
        ShaderOptions ShaderConfig{};

        void SetMaterial(MaterialData const & materialData);
    };

    DirectX::XMFLOAT4X4 m_worldMatrix{};

    std::vector<D3DMeshPart> m_meshParts{};

    Util::BBox m_boundingBox;

    static uint32_t CurrentMeshPartId;
};
