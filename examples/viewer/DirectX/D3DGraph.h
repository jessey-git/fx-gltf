// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
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
        int32_t cameraIndex = -1;
        int32_t meshIndex = -1;
        DirectX::XMMATRIX currentTransform{};
    };

    void Visit(fx::gltf::Document & doc, uint32_t nodeIndex, DirectX::XMMATRIX & parentTransform, std::vector<Node> & graphNodes)
    {
        Node & graphNode = graphNodes[nodeIndex];
        graphNode.currentTransform = parentTransform;

        fx::gltf::Node const & node = doc.nodes[nodeIndex];
        if (node.matrix != fx::gltf::defaults::IdentityMatrix)
        {
            DirectX::XMFLOAT4X4 local(node.matrix.data());
            graphNode.currentTransform = XMLoadFloat4x4(&local) * graphNode.currentTransform;
        }
        else
        {
            if (node.scale != fx::gltf::defaults::IdentityVec3)
            {
                DirectX::XMFLOAT3 local(node.scale.data());
                graphNode.currentTransform = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&local)) * graphNode.currentTransform;
            }

            if (node.rotation != fx::gltf::defaults::IdentityVec4)
            {
                DirectX::XMFLOAT4 local(node.rotation.data());
                graphNode.currentTransform = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&local)) * graphNode.currentTransform;
            }

            if (node.translation != fx::gltf::defaults::NullVec3)
            {
                DirectX::XMFLOAT3 local(node.translation.data());
                graphNode.currentTransform = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&local)) * graphNode.currentTransform;
            }
        }

        if (node.camera >= 0)
        {
            graphNode.cameraIndex = node.camera;
        }
        else
        {
            if (node.mesh >= 0)
            {
                graphNode.meshIndex = node.mesh;
            }

            for (auto childIndex : node.children)
            {
                Visit(doc, childIndex, graphNode.currentTransform, graphNodes);
            }
        }
    }
} // namespace Graph