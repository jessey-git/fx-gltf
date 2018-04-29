// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once
#include <dxgi1_5.h>
#include <fx/gltf.h>

DXGI_FORMAT GetFormat(fx::gltf::Accessor const * accessor)
{
    if (accessor->type == fx::gltf::Accessor::Type::Vec3 && accessor->componentType == fx::gltf::Accessor::ComponentType::Float)
    {
        return DXGI_FORMAT_R32G32B32_FLOAT;
    }
    else if (accessor->type == fx::gltf::Accessor::Type::Scalar && accessor->componentType == fx::gltf::Accessor::ComponentType::UnsignedInt)
    {
        return DXGI_FORMAT_R32_UINT;
    }
    else if (accessor->type == fx::gltf::Accessor::Type::Scalar && accessor->componentType == fx::gltf::Accessor::ComponentType::UnsignedShort)
    {
        return DXGI_FORMAT_R16_UINT;
    }
    else
    {
        throw std::runtime_error("Unknown accessor types");
    }
}
