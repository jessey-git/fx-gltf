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

DirectX::XMFLOAT3 HSVtoRBG(float hue, float saturation, float value)
{
    DirectX::XMFLOAT3 rgb;

    rgb.x = fabsf(hue * 6.0f - 3.0f) - 1.0f;
    rgb.y = 2.0f - fabsf(hue * 6.0f - 2.0f);
    rgb.z = 2.0f - fabsf(hue * 6.0f - 4.0f);

    rgb.x = std::clamp(rgb.x, 0.0f, 1.0f);
    rgb.y = std::clamp(rgb.y, 0.0f, 1.0f);
    rgb.z = std::clamp(rgb.z, 0.0f, 1.0f);

    rgb.x = ((rgb.x - 1.0f) * saturation + 1.0f) * value;
    rgb.y = ((rgb.y - 1.0f) * saturation + 1.0f) * value;
    rgb.z = ((rgb.z - 1.0f) * saturation + 1.0f) * value;

    return rgb;
}
