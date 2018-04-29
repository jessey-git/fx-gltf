// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#include "stdafx.h"
#include "Engine.h"

class Win32Application
{
public:
    static int Run(HINSTANCE hInstance, int nCmdShow)
    {
        // Parse the command line parameters
        int argc;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        LocalFree(argv);

        std::unique_ptr<Engine> engine = std::make_unique<Engine>(960, 540);

        // Initialize the window class.
        WNDCLASSEX windowClass = { 0 };
        windowClass.cbSize = sizeof(WNDCLASSEX);
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = WindowProc;
        windowClass.hInstance = hInstance;
        windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
        windowClass.lpszClassName = L"viewer";
        RegisterClassEx(&windowClass);

        RECT windowRect = { 0, 0, static_cast<LONG>(engine->Width()), static_cast<LONG>(engine->Height()) };
        AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

        // Create the window and store a handle to it.
        HWND hwnd = CreateWindow(
            windowClass.lpszClassName,
            L"viewer",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            nullptr,		// We have no parent window.
            nullptr,		// We aren't using menus.
            hInstance,
            engine.get());

        // Initialize the sample. OnInit is defined in each child-implementation of DXSample.
        engine->OnInit(hwnd);

        ShowWindow(hwnd, nCmdShow);

        // Main sample loop.
        MSG msg = {};
        while (msg.message != WM_QUIT)
        {
            // Process any messages in the queue.
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        engine->OnDestroy();

        // Return this part of the WM_QUIT message to Windows.
        return static_cast<char>(msg.wParam);
    }

private:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        Engine * engine = reinterpret_cast<Engine *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

        switch (message)
        {
        case WM_CREATE:
        {
            LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        }
        return 0;

        case WM_KEYDOWN:
            if (engine)
            {
                engine->OnKeyDown(static_cast<UINT8>(wParam));
            }
            return 0;

        case WM_KEYUP:
            if (engine)
            {
                engine->OnKeyUp(static_cast<UINT8>(wParam));
            }
            return 0;

        case WM_PAINT:
            if (engine)
            {
                engine->Tick();
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }

        // Handle any messages the switch statement didn't.
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int nCmdShow)
{
    return Win32Application::Run(hInstance, nCmdShow);
}
