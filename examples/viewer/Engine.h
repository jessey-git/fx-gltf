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
    Engine(EngineOptions const & config)
        : m_config(config)
    {
    }

    Engine(Engine const &) = delete;
    Engine(Engine &&) = delete;
    Engine & operator=(Engine const &) = delete;
    Engine & operator=(Engine &&) = delete;

    virtual ~Engine() = default;

    void Initialize(HWND hwnd)
    {
        InitializeCore(hwnd);
    }

    void Tick()
    {
        m_timer.Tick([&]() { Update(static_cast<float>(m_timer.GetElapsedSeconds())); });

        Render();
    }

    void WindowSizeChanged(int width, int height)
    {
        WindowSizeChangedCore(width, height);
    }

protected:
    EngineOptions const & Config() noexcept { return m_config; }

    virtual void InitializeCore(HWND hwnd) = 0;
    virtual void Update(float elapsedTime) = 0;
    virtual void Render() = 0;
    virtual void WindowSizeChangedCore(int width, int height) = 0;

private:
    EngineOptions m_config{};

    StepTimer m_timer{};
};
