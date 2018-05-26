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
        : Config(config)
    {
    }

    Engine(Engine const &) = delete;
    Engine(Engine &&) = delete;
    Engine & operator=(Engine const &) = delete;
    Engine & operator=(Engine &&) = delete;

    virtual ~Engine() = default;

    const EngineOptions Config;

    void Initialize(HWND hwnd)
    {
        InitializeCore(hwnd);
    }

    void Tick()
    {
        m_timer.Tick([&]() { Update(static_cast<float>(m_timer.GetElapsedSeconds())); });

        Render();
    }

    void ChangeWindowSize(int width, int height)
    {
        ChangeWindowSizeCore(width, height);
    }

protected:
    virtual void InitializeCore(HWND hwnd) = 0;
    virtual void Update(float elapsedTime) noexcept = 0;
    virtual void Render() = 0;
    virtual void ChangeWindowSizeCore(int width, int height) = 0;

private:
    StepTimer m_timer{};
};
