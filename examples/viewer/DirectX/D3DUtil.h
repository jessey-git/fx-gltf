// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <algorithm>
#include <cstdio>
#include <dxgi1_5.h>
#include <exception>
#include <fx/gltf.h>

namespace Util
{
    struct BBox
    {
        DirectX::XMFLOAT3 min{};
        DirectX::XMFLOAT3 max{};
        DirectX::XMFLOAT3 centerTranslation{};
    };

    static DXGI_FORMAT GetFormat(fx::gltf::Accessor const * accessor)
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

    static void AdjustBBox(BBox & currentBBox, BBox const & other) noexcept
    {
        currentBBox.min.x = std::min(currentBBox.min.x, other.min.x);
        currentBBox.min.y = std::min(currentBBox.min.y, other.min.y);
        currentBBox.min.z = std::min(currentBBox.min.z, other.min.z);

        currentBBox.max.x = std::max(currentBBox.max.x, other.max.x);
        currentBBox.max.y = std::max(currentBBox.max.y, other.max.y);
        currentBBox.max.z = std::max(currentBBox.max.z, other.max.z);
    }

    static void CenterBBox(BBox & currentBBox)
    {
        using namespace DirectX;
        const DirectX::XMVECTOR min = DirectX::XMLoadFloat3(&currentBBox.min);
        const DirectX::XMVECTOR max = DirectX::XMLoadFloat3(&currentBBox.max);
        const DirectX::XMVECTOR mid = DirectX::XMVectorNegate(0.5f * (min + max));

        DirectX::XMStoreFloat3(&currentBBox.centerTranslation, mid);
    }

    static DirectX::XMFLOAT4 HSVtoRBG(float hue, float saturation, float value) noexcept
    {
        DirectX::XMFLOAT4 rgba{};

        rgba.x = fabsf(hue * 6.0f - 3.0f) - 1.0f;
        rgba.y = 2.0f - fabsf(hue * 6.0f - 2.0f);
        rgba.z = 2.0f - fabsf(hue * 6.0f - 4.0f);

        rgba.x = std::clamp(rgba.x, 0.0f, 1.0f);
        rgba.y = std::clamp(rgba.y, 0.0f, 1.0f);
        rgba.z = std::clamp(rgba.z, 0.0f, 1.0f);

        rgba.x = ((rgba.x - 1.0f) * saturation + 1.0f) * value;
        rgba.y = ((rgba.y - 1.0f) * saturation + 1.0f) * value;
        rgba.z = ((rgba.z - 1.0f) * saturation + 1.0f) * value;

        rgba.w = 1.0f;
        return rgba;
    }
} // namespace Util

namespace DX
{
    // Helper class for COM exceptions
    class com_exception : public std::exception
    {
    public:
        explicit com_exception(HRESULT hr) noexcept : result(hr) {}

        const char* what() const noexcept override
        {
            static char s_str[64] = {};
            sprintf_s(s_str, "Failure with HRESULT of %08X", result);
            return s_str;
        }

    private:
        HRESULT result;
    };

    // Helper utility converts D3D API failures into exceptions.
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw com_exception(hr);
        }
    }
} // namespace DX
