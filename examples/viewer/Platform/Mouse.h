// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once

#include <memory>

class Mouse
{
public:
    Mouse() noexcept(false);
    Mouse(Mouse && moveFrom) noexcept;
    Mouse & operator=(Mouse && moveFrom) noexcept;

    Mouse(Mouse const &) = delete;
    Mouse & operator=(Mouse const &) = delete;

    virtual ~Mouse();

    enum Mode
    {
        MODE_ABSOLUTE = 0,
        MODE_RELATIVE,
    };

    struct State
    {
        bool leftButton;
        bool middleButton;
        bool rightButton;
        bool xButton1;
        bool xButton2;
        int x;
        int y;
        int scrollWheelValue;
        Mode positionMode;
    };

    class ButtonStateTracker
    {
    public:
        enum ButtonState
        {
            UP = 0, // Button is up
            HELD = 1, // Button is held down
            RELEASED = 2, // Button was just released
            PRESSED = 3, // Buton was just pressed
        };

        ButtonState leftButton{};
        ButtonState middleButton{};
        ButtonState rightButton{};
        ButtonState xButton1{};
        ButtonState xButton2{};

        ButtonStateTracker() noexcept
        {
            Reset();
        }

        void __cdecl Update(const State & state) noexcept;

        void __cdecl Reset() noexcept;

        State __cdecl GetLastState() const noexcept
        {
            return lastState;
        }

    private:
        State lastState{};
    };

    // Retrieve the current state of the mouse
    State __cdecl GetState() const;

    // Resets the accumulated scroll wheel value
    void __cdecl ResetScrollWheelValue() noexcept;

    // Sets mouse mode (defaults to absolute)
    void __cdecl SetMode(Mode mode);

    // Feature detection
    bool __cdecl IsConnected() const noexcept;

    // Cursor visibility
    bool __cdecl IsVisible() const;
    void __cdecl SetVisible(bool visible);

    void __cdecl SetWindow(HWND window);
    static void __cdecl ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam);

    // Singleton
    static Mouse & __cdecl Get();

private:
    class Impl;

    std::unique_ptr<Impl> pImpl;
};
