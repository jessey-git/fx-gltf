#include "stdafx.h"

#include "D3DOrbitCamera.h"

void D3DOrbitCamera::Reset(DirectX::XMFLOAT3 position)
{
    constexpr DirectX::XMVECTOR zAxis = { { 0, 0, 1, 0 } };
    constexpr DirectX::XMVECTOR yAxis = { { 0, 1, 0, 0 } };

    position.y = -position.y;
    const DirectX::XMVECTOR original = DirectX::XMLoadFloat3(&position);

    position.y = 0;
    const DirectX::XMVECTOR projected = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&position));

    const int xFlip = position.x < 0 ? -1 : 1;

    Reset(
        DirectX::XMVectorGetX(DirectX::XMVector3Length(original)),
        DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(DirectX::XMVector3Normalize(original), yAxis)),
        xFlip * DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(projected, zAxis)));
}

void D3DOrbitCamera::Reset(float radius, float phi, float theta)
{
    m_radius = 1;
    m_phi = DirectX::XM_PIDIV2;
    m_theta = 0;

    Update(radius, phi, theta);
}

void D3DOrbitCamera::Update(float zoomFactor, float deltaPhi, float deltaTheta)
{
    Dolly(zoomFactor);
    RotateLeft(deltaTheta);
    RotateUp(deltaPhi);

    Calculate();
}

void D3DOrbitCamera::Update(Mouse::ButtonStateTracker tracker)
{
    using ButtonState = Mouse::ButtonStateTracker::ButtonState;

    // Query mouse state...
    bool changed = false;
    const Mouse::State state = tracker.GetLastState();
    if (tracker.leftButton == ButtonState::PRESSED || tracker.middleButton == ButtonState::PRESSED)
    {
        TrackLastCursorPosition(state);
    }
    else if (tracker.leftButton == ButtonState::HELD)
    {
        RotateLeft(DirectX::XM_2PI * (state.x - m_lastCursorPos.x) / 540.0f);
        RotateUp(DirectX::XM_2PI * (state.y - m_lastCursorPos.y) / 540.0f);

        changed = true;
        TrackLastCursorPosition(state);
    }
    else if (tracker.middleButton == ButtonState::HELD)
    {
        const int deltaY = state.y - m_lastCursorPos.y;
        if (deltaY < 0)
        {
            Dolly(0.95f);
        }
        else if (deltaY > 0)
        {
            Dolly(1.0f / 0.95f);
        }

        changed = true;
        TrackLastCursorPosition(state);
    }

    if (changed)
    {
        Calculate();
    }
}

void D3DOrbitCamera::SetProjection(float fovAngleY, float aspectRatio, float nearZ, float farZ)
{
    const DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, nearZ, farZ);
    const DirectX::XMMATRIX viewProj = DirectX::XMLoadFloat4x4(&ViewMatrix) * projection;

    DirectX::XMStoreFloat4x4(&m_projectionMatrix, projection);
    DirectX::XMStoreFloat4x4(&ViewProjectionMatrix, viewProj);
}

void D3DOrbitCamera::Calculate()
{
    constexpr DirectX::XMVECTOR Backward = { { 0.0f, 0.0f, 1.0f, 0.0f } };
    constexpr DirectX::XMVECTOR At = { { 0.0f, 0.0f, 0.0f, 0.0f } };
    constexpr DirectX::XMVECTOR Up = { { 0.0f, 1.0f, 0.0f, 0.0 } };

    // Update camera position...
    Position = DirectX::XMVector3Transform(Backward, DirectX::XMMatrixRotationRollPitchYaw(m_phi, m_theta, 0.0f));
    Position = DirectX::XMVectorScale(Position, m_radius);

    // Refresh view and view+projection matrices...
    DirectX::XMStoreFloat4x4(&ViewMatrix, DirectX::XMMatrixLookAtLH(Position, At, Up));
    DirectX::XMStoreFloat4x4(&ViewProjectionMatrix, DirectX::XMLoadFloat4x4(&ViewMatrix) * DirectX::XMLoadFloat4x4(&m_projectionMatrix));
}

void D3DOrbitCamera::TrackLastCursorPosition(Mouse::State const & state) noexcept
{
    m_lastCursorPos.x = state.x;
    m_lastCursorPos.y = state.y;
}

void D3DOrbitCamera::Dolly(float zoomFactor) noexcept
{
    m_radius *= zoomFactor;
}

void D3DOrbitCamera::RotateLeft(float deltaTheta) noexcept
{
    m_theta += deltaTheta;
}

void D3DOrbitCamera::RotateUp(float deltaPhi) noexcept
{
    constexpr float Epsilon = 0.000001f;

    m_phi -= deltaPhi;
    m_phi = std::clamp(m_phi, -DirectX::XM_PIDIV2 + Epsilon, DirectX::XM_PIDIV2 - Epsilon);
}
