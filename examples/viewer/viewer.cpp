// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#include "stdafx.h"
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "CLI11/CLI11.hpp"
#include "DirectX/D3DEngine.h"
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

        // Initialize the sample. OnInit is defined in each child-implementation of DXSample.
        engine->Initialize(hwnd);

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

        case WM_PAINT:
            if (engine)
            {
                engine->Tick();
            }

            return 0;

        case WM_SIZE:
            if (engine)
            {
                engine->ChangeWindowSize(LOWORD(lParam), HIWORD(lParam));
            }

            return 0;

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

        CLI::App app{ "viewer.exe" };
        app.add_option("--width", options.Width, "Initial window width");
        app.add_option("--height", options.Height, "Initial window height");
        app.add_flag("-r,--rotate", options.AutoRotate, "Auto rotate model");
        app.add_flag("-m,--materials", options.EnableMaterials, "Enable model materials");
        app.add_flag("-i,--ibl", options.EnableIBL, "Enable IBL");
        app.add_flag("-g,--ground", options.EnableGround, "Enable ground plane");
        app.add_option("-x", options.CameraX, "Camera x position");
        app.add_option("-y", options.CameraY, "Camera y position");
        app.add_option("-z", options.CameraZ, "Camera z position");
        app.add_option("file", options.ModelPath, "Model to load (.gltf or .glb)")->required(true);

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
        catch (CLI::CallForHelp)
        {
            showHelp = true;
        }
        catch (CLI::ParseError & e)
        {
            errorMessage = e.what();
            showHelp = true;
        }

        if (showHelp)
        {
            std::cout << app.help(25) << std::endl << std::endl;
            if (!errorMessage.empty())
            {
                std::cout << errorMessage << std::endl;
            }

            return false;
        }

        return true;
    }
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    FILE * attachedOut{};
    FILE * attachedIn{};
    if (AllocConsole() == TRUE)
    {
        freopen_s(&attachedOut, "CONOUT$", "w", stdout);
        freopen_s(&attachedIn, "CONIN$", "r", stdin);
    }

    CoInitializeEx(nullptr, 0);

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

    if (attachedOut != nullptr)
    {
        fclose(attachedOut);
    }

    if (attachedIn != nullptr)
    {
        fclose(attachedIn);
    }

    return result;
}
