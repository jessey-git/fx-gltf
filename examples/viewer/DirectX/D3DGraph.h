// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <DirectXMath.h>
#include <fx/gltf.h>

namespace Graph
{
    struct Node
    {
        DirectX::XMMATRIX CurrentTransform{};
        int32_t CameraIndex = -1;
        int32_t MeshIndex = -1;
    };

    void Visit(fx::gltf::Document const & doc, uint32_t nodeIndex, DirectX::XMMATRIX const & parentTransform, std::vector<Node> & graphNodes)
    {
        Node & graphNode = graphNodes[nodeIndex];
        graphNode.CurrentTransform = parentTransform;

        fx::gltf::Node const & node = doc.nodes[nodeIndex];
        if (node.matrix != fx::gltf::defaults::IdentityMatrix)
        {
            const DirectX::XMFLOAT4X4 local(node.matrix.data());
            graphNode.CurrentTransform = DirectX::XMLoadFloat4x4(&local) * graphNode.CurrentTransform;
        }
        else
        {
            if (node.translation != fx::gltf::defaults::NullVec3)
            {
                const DirectX::XMFLOAT3 local(node.translation.data());
                graphNode.CurrentTransform = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&local)) * graphNode.CurrentTransform;
            }

            if (node.scale != fx::gltf::defaults::IdentityVec3)
            {
                const DirectX::XMFLOAT3 local(node.scale.data());
                graphNode.CurrentTransform = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&local)) * graphNode.CurrentTransform;
            }

            if (node.rotation != fx::gltf::defaults::IdentityVec4)
            {
                const DirectX::XMFLOAT4 local(node.rotation.data());
                graphNode.CurrentTransform = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&local)) * graphNode.CurrentTransform;
            }
        }

        if (node.camera >= 0)
        {
            graphNode.CameraIndex = node.camera;
        }
        else
        {
            if (node.mesh >= 0)
            {
                graphNode.MeshIndex = node.mesh;
            }

            for (auto childIndex : node.children)
            {
                Visit(doc, childIndex, graphNode.CurrentTransform, graphNodes);
            }
        }
    }
} // namespace Graph
