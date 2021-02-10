// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <DirectXMath.h>
#include "Platform/Mouse.h"

class D3DOrbitCamera
{
public:
    void Reset(DirectX::XMFLOAT3 position);
    void Reset(float radius, float phi, float theta);

    void Update(Mouse::ButtonStateTracker tracker);
    void Update(float zoomFactor, float deltaPhi, float deltaTheta);

    void SetProjection(float fovAngleY, float aspectRatio, float nearZ, float farZ);

    DirectX::XMFLOAT4X4 ViewMatrix{};
    DirectX::XMFLOAT4X4 ViewProjectionMatrix{};
    DirectX::XMVECTOR Position{};

private:
    DirectX::XMFLOAT4X4 m_projectionMatrix{};

    float m_radius{};
    float m_phi{};
    float m_theta{};

    DirectX::XMINT2 m_lastCursorPos{};
    void TrackLastCursorPosition(Mouse::State const & state) noexcept;

    void Calculate();

    void Dolly(float zoomFactor) noexcept;
    void RotateLeft(float deltaTheta) noexcept;
    void RotateUp(float deltaPhi) noexcept;
};
