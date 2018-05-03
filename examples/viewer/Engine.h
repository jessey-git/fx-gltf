// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include "DirectX/D3DEngine.h"
#include "EngineOptions.h"
#include "StepTimer.h"
#include "platform.h"

class Engine
{
public:
    Engine(EngineOptions const & options)
        : m_width(options.Width), m_height(options.Height)
    {
        m_scene = std::make_unique<D3DEngine>(options);
    }

    Engine(Engine const &) = delete;
    Engine(Engine &&) = delete;
    Engine & operator=(Engine const &) = delete;
    Engine & operator=(Engine &&) = delete;

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

    void OnResize(UINT width, UINT height)
    {
        m_width = width;
        m_height = height;
        m_scene->WindowSizeChanged(m_width, m_height);
    }

    void OnDestroy()
    {
        m_scene->Shutdown();
    }

    void OnKeyDown(UINT8) {}
    void OnKeyUp(UINT8) {}

private:
    UINT m_width{};
    UINT m_height{};

    std::unique_ptr<D3DEngine> m_scene{};
    StepTimer m_timer{};
};
