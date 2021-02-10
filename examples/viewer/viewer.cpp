// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#include "stdafx.h"
#include <fx/gltf.h>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#define CLI11_BOOST_OPTIONAL 0
#include "CLI11/CLI11.hpp"

#include "DirectX/D3DEngine.h"
#include "Engine.h"
#include "EngineOptions.h"
#include "Platform/COMUtil.h"
#include "Platform/Mouse.h"

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

        std::unique_ptr<Engine> engine = std::make_unique<D3DEngine>(options);

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

        // Initialize the engine...
        engine->Initialize(hwnd);

        ShowWindow(hwnd, nCmdShow);

        // Main loop.
        MSG msg{};
        while (msg.message != WM_QUIT)
        {
            // Process any messages in the queue.
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

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

                Mouse::ProcessMessage(message, wParam, lParam);
            }

            return 0;

        case WM_PAINT:
            if (engine != nullptr)
            {
                engine->Tick();
            }

            return 0;

        case WM_SIZE:
            if (engine != nullptr)
            {
                engine->ChangeWindowSize(LOWORD(lParam), HIWORD(lParam));
            }

            return 0;

        case WM_INPUT:
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MOUSEWHEEL:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
        case WM_MOUSEHOVER:
            Mouse::ProcessMessage(message, wParam, lParam);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }

        // Handle any messages the switch statement didn't.
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    static bool TryParseCommandLine(EngineOptions & options)
    {
        bool showHelp{};

        CLI::App app{ "A simple glTF2.0 scene viewer using DirectX 12", "viewer.exe" };
        app.add_option("--width", options.Width, "Initial window width");
        app.add_option("--height", options.Height, "Initial window height");
        app.add_flag("-r,--rotate", options.AutoRotate, "Auto rotate model");
        app.add_flag("-m,--materials", options.EnableMaterials, "Enable model materials");
        app.add_flag("-i,--ibl", options.EnableIBL, "Enable IBL");
        app.add_flag("-g,--ground", options.EnableGround, "Enable ground plane");
        app.add_option("-x", options.CameraX, "Camera x position");
        app.add_option("-y", options.CameraY, "Camera y position");
        app.add_option("-z", options.CameraZ, "Camera z position");
        app.add_option("file", options.ModelPath, "Scene to load (.gltf or .glb)")->required(true);

        int argc;
        std::vector<std::string> args{};
        LPWSTR * argv = CommandLineToArgvW(GetCommandLine(), &argc);
        if (argv != nullptr && argc > 0)
        {
            for (int i = argc - 1; i > 0; i--)
            {
                std::string arg;
                for (auto c : std::wstring_view(argv[i]))
                {
                    arg.push_back(static_cast<char>(c));
                }

                args.emplace_back(std::move(arg));
            }
            LocalFree(argv);
        }
        else
        {
            showHelp = true;
        }

        std::string errorMessage{};
        try
        {
            app.parse(args);
        }
        catch (CLI::CallForHelp const &)
        {
            showHelp = true;
        }
        catch (CLI::ParseError const & e)
        {
            errorMessage = e.what();
            showHelp = true;
        }

        if (showHelp)
        {
            std::cout << app.help() << std::endl;
            std::cout << "Controls:" << std::endl;
            std::cout << "  Orbit with left mouse button" << std::endl;
            std::cout << "  Dolly with middle mouse button" << std::endl;
            std::cout << std::endl;

            if (!errorMessage.empty())
            {
                std::cout << errorMessage << std::endl;
            }

            return false;
        }

        return true;
    }
};

int wmain()
{
    int result = -1;
    try
    {
        COMUtil::Init();
        result = Win32Application::Run(GetModuleHandle(nullptr), SW_SHOWDEFAULT);
    }
    catch (std::exception const & e)
    {
        std::string message;
        fx::FormatException(message, e);
        std::cout << message << std::endl;
    }

    return result;
}
