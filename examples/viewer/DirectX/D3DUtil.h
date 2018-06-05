// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <algorithm>
#include <cstdio>
#include <d3dcompiler.h>
#include <dxgi1_5.h>
#include <exception>
#include <fx/gltf.h>
#include <wrl.h>

#include "Logger.h"

namespace Util
{
    struct BBox
    {
        DirectX::XMFLOAT3 Min{};
        DirectX::XMFLOAT3 Max{};
        DirectX::XMFLOAT3 CenterTranslation{};
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

    static uint64_t ResourceSize(uint32_t size) noexcept
    {
        const std::size_t MinResourceSize = 64 * 1024;
        return size < MinResourceSize ? MinResourceSize : size;
    }

    static void CenterBBox(BBox & currentBBox)
    {
        using namespace DirectX;
        const DirectX::XMVECTOR min = DirectX::XMLoadFloat3(&currentBBox.Min);
        const DirectX::XMVECTOR max = DirectX::XMLoadFloat3(&currentBBox.Max);
        const DirectX::XMVECTOR mid = DirectX::XMVectorNegate(0.5f * (min + max));

        DirectX::XMStoreFloat3(&currentBBox.CenterTranslation, mid);
    }

    static void UnionBBox(BBox & currentBBox, BBox const & other) noexcept
    {
        const DirectX::XMVECTOR cMin = DirectX::XMLoadFloat3(&currentBBox.Min);
        const DirectX::XMVECTOR cMax = DirectX::XMLoadFloat3(&currentBBox.Max);
        const DirectX::XMVECTOR oMin = DirectX::XMLoadFloat3(&other.Min);
        const DirectX::XMVECTOR oMax = DirectX::XMLoadFloat3(&other.Max);
        DirectX::XMStoreFloat3(&currentBBox.Min, DirectX::XMVectorMin(cMin, oMin));
        DirectX::XMStoreFloat3(&currentBBox.Max, DirectX::XMVectorMax(cMax, oMax));

        Util::CenterBBox(currentBBox);
    }

    static BBox TransformBBox(BBox const & currentBBox, DirectX::XMMATRIX const & transform)
    {
        using namespace DirectX;
        const DirectX::XMVECTOR newMin = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&currentBBox.Min), transform);
        const DirectX::XMVECTOR newMax = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&currentBBox.Max), transform);

        BBox newBBox{};
        DirectX::XMStoreFloat3(&newBBox.Min, newMin);
        DirectX::XMStoreFloat3(&newBBox.Max, newMax);
        Util::CenterBBox(newBBox);

        return newBBox;
    }

    static BBox TransformBBox(BBox const & currentBBox, DirectX::XMFLOAT3 const & centerTranslation, float scalingFactor)
    {
        using namespace DirectX;
        const DirectX::XMMATRIX translation = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&centerTranslation));
        const DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(scalingFactor, scalingFactor, scalingFactor);

        return Util::TransformBBox(currentBBox, translation * scale);
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
        explicit com_exception(HRESULT hr) noexcept
            : result(hr) {}

        const char * what() const noexcept override
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

    inline Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
        std::wstring const & filename,
        std::string const & entrypoint,
        std::string const & target,
        D3D_SHADER_MACRO const * defines)
    {
        UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
        compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

        Microsoft::WRL::ComPtr<ID3DBlob> byteCode{};
        Microsoft::WRL::ComPtr<ID3DBlob> errors{};
        const HRESULT hr =
            D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

        if (errors != nullptr)
        {
            Logger::WriteLine(static_cast<char *>(errors->GetBufferPointer()));
        }

        ThrowIfFailed(hr);
        return byteCode;
    }
} // namespace DX
