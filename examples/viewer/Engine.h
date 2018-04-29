// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include "DirectX/D3DEngine.h"
#include "StepTimer.h"
#include "platform.h"

class Engine
{
public:
    Engine(UINT width, UINT height)
        : m_width(width), m_height(height)
    {
        m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);

        m_scene = std::make_unique<D3DEngine>();
    }

    ~Engine() = default;

    void OnInit(HWND hwnd)
    {
        m_scene->Initialize(hwnd, m_width, m_height);
    }

    void Tick()
    {
        m_timer.Tick([&]() { m_scene->Update(float(m_timer.GetElapsedSeconds())); });

        m_scene->Render();
    }

    void OnDestroy() {}

    void OnKeyDown(UINT8) {}
    void OnKeyUp(UINT8) {}

    UINT Width() const
    {
        return m_width;
    }

    UINT Height() const
    {
        return m_height;
    }

private:
    UINT m_width;
    UINT m_height;
    float m_aspectRatio;

    std::unique_ptr<D3DEngine> m_scene;
    DX::StepTimer m_timer;
};
