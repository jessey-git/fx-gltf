// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#include "stdafx.h"
#include <string>
#include <vector>
#include <iostream>

#include "clara/clara.hpp"
#include "Engine.h"
#include "EngineOptions.h"

class Win32Application
{
public:
    static int Run(HINSTANCE hInstance, int nCmdShow)
    {
        EngineOptions options{};
        if (!TryParseCommandLine(options))
        {
            return -1;
        }

        std::unique_ptr<Engine> engine = std::make_unique<Engine>(options);

        // Initialize the window class.
        WNDCLASSEX windowClass = { 0 };
        windowClass.cbSize = sizeof(WNDCLASSEX);
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = WindowProc;
        windowClass.hInstance = hInstance;
        windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
        windowClass.lpszClassName = L"viewer";
        RegisterClassEx(&windowClass);

        RECT windowRect = { 0, 0, static_cast<LONG>(options.Width), static_cast<LONG>(options.Height) };
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
            nullptr, // We have no parent window.
            nullptr, // We aren't using menus.
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
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        engine->OnDestroy();

        // Return this part of the WM_QUIT message to Windows.
        return static_cast<char>(msg.wParam);
    }

    static bool TryParseCommandLine(EngineOptions & options)
    {
        bool showHelp{};
        auto cli
            = clara::Help(showHelp)
            | clara::Opt(options.Width, "width")
                ["--width"]("Initial window width")
            | clara::Opt(options.Height, "height")
                ["--height"]("Initial window height")
            | clara::Opt(options.AutoRotate)
                ["-r"]["--rotate"]("Auto rotate model")
            | clara::Arg(options.ModelPath, "model")
                ("Model to load").required(); // Clara does not enforce required arguments right now :-/

        int argc;
        LPWSTR * argv = CommandLineToArgvW(GetCommandLine(), &argc);
        std::vector<std::string> args(argc - 1);
        for (int i = 1; i < argc; i++)
        {
            std::wstring arg(argv[i]);
            for (auto c : arg)
            {
                args[i - 1].push_back(static_cast<char>(c));
            }
        }
        LocalFree(argv);

        auto results = cli.parse("viewer.exe", clara::detail::TokenStream(args.begin(), args.end()));
        if (showHelp)
        {
            std::cout << cli << std::endl << std::endl;
            return false;
        }

        if (!results)
        {
            throw std::runtime_error(results.errorMessage().c_str());
        }
        else if (options.ModelPath.empty())
        {
            throw std::runtime_error("A model path must be provided");
        }

        return true;
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

        case WM_SIZE:
            if (engine)
            {
                engine->OnResize(LOWORD(lParam), HIWORD(lParam));
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

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    FILE * attachedOut = nullptr;
    FILE * attachedIn = nullptr;
    if (AllocConsole() == TRUE)
    {
        freopen_s(&attachedOut, "CONOUT$", "w", stdout);
        freopen_s(&attachedIn, "CONIN$", "r", stdin);
    }

    int result = -1;
    try
    {
        result = Win32Application::Run(hInstance, nCmdShow);
    }
    catch (std::exception & e)
    {
        std::cout << e.what() << std::endl;
    }

    if (result < 0)
    {
        std::cout << "Press [ENTER] to continue" << std::endl;
        std::cin.get();
    }

    fclose(attachedOut);
    fclose(attachedIn);

    return result;
}
