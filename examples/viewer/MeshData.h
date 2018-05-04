// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <fx/gltf.h>

class MeshData
{
public:
    struct BufferInfo
    {
        fx::gltf::Accessor const * accessor;

        uint8_t const * data;
        uint32_t dataStride;
        uint32_t totalSize;
    };

    MeshData(fx::gltf::Document const & doc, std::size_t meshIndex, std::size_t primitveIndex)
    {
        fx::gltf::Mesh const & mesh = doc.meshes[meshIndex];
        fx::gltf::Primitive const & primitive = mesh.primitives[primitveIndex];

        for (auto const & attrib : primitive.attributes)
        {
            if (attrib.first == "POSITION")
            {
                vertexBuffer = GetData(doc, doc.accessors[attrib.second]);
            }
            else if (attrib.first == "NORMAL")
            {
                normalBuffer = GetData(doc, doc.accessors[attrib.second]);
            }
        }

        indexBuffer = GetData(doc, doc.accessors[primitive.indices]);
    }

    bool HasIndexData() noexcept
    {
        return indexBuffer.data != nullptr;
    }

    bool HasVertexData() noexcept
    {
        return vertexBuffer.data != nullptr;
    }

    bool HasNormalData() noexcept
    {
        return normalBuffer.data != nullptr;
    }

    BufferInfo IndexBuffer() noexcept
    {
        return indexBuffer;
    }

    BufferInfo VertexBuffer() noexcept
    {
        return vertexBuffer;
    }

    BufferInfo NormalBuffer() noexcept
    {
        return normalBuffer;
    }

private:
    BufferInfo indexBuffer{};
    BufferInfo vertexBuffer{};
    BufferInfo normalBuffer{};

    static BufferInfo GetData(fx::gltf::Document const & doc, fx::gltf::Accessor const & accessor)
    {
        fx::gltf::BufferView const & bufferView = doc.bufferViews[accessor.bufferView];
        fx::gltf::Buffer const & buffer = doc.buffers[bufferView.buffer];

        const uint32_t dataTypeSize = CalculateDataTypeSize(accessor);
        return BufferInfo{ &accessor, &buffer.data[static_cast<uint64_t>(bufferView.byteOffset) + accessor.byteOffset], dataTypeSize, accessor.count * dataTypeSize };
    }

    static uint32_t CalculateDataTypeSize(fx::gltf::Accessor const & accessor) noexcept
    {
        uint32_t elementSize = 0;
        switch (accessor.componentType)
        {
        case fx::gltf::Accessor::ComponentType::Byte:
        case fx::gltf::Accessor::ComponentType::UnsignedByte:
            elementSize = 1;
            break;
        case fx::gltf::Accessor::ComponentType::Short:
        case fx::gltf::Accessor::ComponentType::UnsignedShort:
            elementSize = 2;
            break;
        case fx::gltf::Accessor::ComponentType::Float:
        case fx::gltf::Accessor::ComponentType::UnsignedInt:
            elementSize = 4;
            break;
        }

        switch (accessor.type)
        {
        case fx::gltf::Accessor::Type::Mat2:
            return 4 * elementSize;
            break;
        case fx::gltf::Accessor::Type::Mat3:
            return 9 * elementSize;
            break;
        case fx::gltf::Accessor::Type::Mat4:
            return 16 * elementSize;
            break;
        case fx::gltf::Accessor::Type::Scalar:
            return elementSize;
            break;
        case fx::gltf::Accessor::Type::Vec2:
            return 2 * elementSize;
            break;
        case fx::gltf::Accessor::Type::Vec3:
            return 3 * elementSize;
            break;
        case fx::gltf::Accessor::Type::Vec4:
            return 4 * elementSize;
            break;
        }

        return 0;
    }
};
