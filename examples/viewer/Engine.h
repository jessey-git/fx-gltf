// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <string>

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
        const int MaxTextLength = 32;
        wchar_t currentWindowText[MaxTextLength] = { 0 };
        GetWindowText(hwnd, currentWindowText, MaxTextLength);

        m_window = hwnd;
        m_windowTitle.assign(currentWindowText);

        InitializeCore(hwnd);
    }

    void Tick()
    {
        UpdateStats();

        m_timer.Tick([this]() { Update(static_cast<float>(m_timer.GetElapsedSeconds())); });

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

    HWND m_window{};
    std::wstring m_windowTitle{};

    void UpdateStats()
    {
        static double prevTime = 0.0f;

        const double currentTime = m_timer.GetTotalSeconds();
        if ((currentTime - prevTime) >= 1.0f)
        {
            std::wstring fps = std::to_wstring(m_timer.GetFramesPerSecond());
            std::wstring windowText = m_windowTitle + L" : " + fps + L" fps";

            SetWindowText(m_window, windowText.c_str());

            prevTime = currentTime;
        }
    }
};
