#include "OS-ImGui_External.h"
#include "DiscordOverlay.h"

/****************************************************
* Copyright (C)	: Liv
* @file			: OS-ImGui_External.cpp
* @author		: Liv
* @email		: 1319923129@qq.com
* @version		: 1.0
* @date			: 2024/3/31 12:54
****************************************************/

// D3D11 Device
namespace OSImGui
{
#ifdef _CONSOLE
    bool D3DDevice::CreateDeviceD3D(HWND hWnd)
    {
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 2;
        sd.BufferDesc.Width = 0;
        sd.BufferDesc.Height = 0;
        sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        UINT createDeviceFlags = 0;
        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
        HRESULT res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
        if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
            res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_WARP, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
        if (res != S_OK)
            return false;

        CreateRenderTarget();
        return true;
    }

    void D3DDevice::CleanupDeviceD3D()
    {
        CleanupRenderTarget();
        if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
        if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
        if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    }

    void D3DDevice::CreateRenderTarget()
    {
        ID3D11Texture2D* pBackBuffer;
        g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (pBackBuffer == nullptr)
            return;
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }

    void D3DDevice::CleanupRenderTarget()
    {
        if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
    }
#endif
}

// OSImGui External
namespace OSImGui
{
    void  OSImGui_External::AttachAnotherWindow(std::string DestWindowName, std::string DestWindowClassName, std::function<void()> CallBack)
    {
        if (!CallBack)
            throw OSException("CallBack is empty");
        if (DestWindowName.empty() && DestWindowClassName.empty())
            throw OSException("DestWindowName and DestWindowClassName are empty");

        Window.BgColor = ImColor(0, 0, 0, 0);

        DestWindow.hWnd = FindWindowA(
            (DestWindowClassName.empty() ? NULL : DestWindowClassName.c_str()),
            (DestWindowName.empty() ? NULL : DestWindowName.c_str()));
        if (DestWindow.hWnd == NULL)
            throw OSException("DestWindow isn't exist");
        DestWindow.Name = DestWindowName;
        DestWindow.ClassName = DestWindowClassName;

        DWORD ProcessID;
        GetWindowThreadProcessId(DestWindow.hWnd, reinterpret_cast<LPDWORD>(&ProcessID));

        if (ProcessID == 0)
            throw OSException("DestProcess isn't exist");

        if(!DiscordOverlay::DiscordCom.Init(ProcessID))
            throw OSException("Discord Communication init failed");

        Window.hWnd = DestWindow.hWnd;

        Type = ATTACH;
        CallBackFn = CallBack;

        if (!CreateDevice())
            throw OSException("CreateMyWindow() call failed");

        try {
            InitImGui(g_Device.g_pd3dDevice,g_Device.g_pd3dDeviceContext);
        }
        catch (OSException& e)
        {
            throw e;
        }

        MainLoop();
    }

    bool OSImGui_External::PeekEndMessage()
    {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                return true;
        }
        return false;
    }

    void OSImGui_External::MainLoop()
    {
        while (!EndFlag)
        {
            if (PeekEndMessage())
                break;
            if (Type == ATTACH)
            {
                if (!UpdateWindowData())
                    break;
            }

            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            this->CallBackFn();

            ImGui::Render();
            const float clear_color_with_alpha[4] = { Window.BgColor.Value.x, Window.BgColor.Value.y , Window.BgColor.Value.z, Window.BgColor.Value.w };
            g_Device.g_pd3dDeviceContext->OMSetRenderTargets(1, &g_Device.g_mainRenderTargetView, NULL);
            g_Device.g_pd3dDeviceContext->ClearRenderTargetView(g_Device.g_mainRenderTargetView, clear_color_with_alpha);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            
            // Send render buffers.
            DiscordOverlay::DiscordCom.SendBuffer(g_Device.g_pd3dDeviceContext, g_Device.g_pSwapChain, ImGui::GetMainViewport()->Size);
        }
        DiscordOverlay::DiscordCom.Stop();
        CleanImGui();
    }

    bool OSImGui_External::CreateDevice()
    {
        if (!g_Device.CreateDeviceD3D(DestWindow.hWnd))
        {
            g_Device.CleanupDeviceD3D();
            return false;
        }

        return true;
    }

    bool OSImGui_External::UpdateWindowData()
    {
        POINT Point{};
        RECT Rect{};

        DestWindow.hWnd = FindWindowA(
            (DestWindow.ClassName.empty() ? NULL : DestWindow.ClassName.c_str()),
            (DestWindow.Name.empty() ? NULL : DestWindow.Name.c_str()));
        if (DestWindow.hWnd == NULL)
            return false;

        GetClientRect(DestWindow.hWnd, &Rect);
        ClientToScreen(DestWindow.hWnd, &Point);
        
        // Resize buffers.
        if (Rect.right != Window.Size.x || Rect.bottom != Window.Size.y)
        {
            if (g_Device.g_pd3dDevice != NULL)
            {
                g_Device.CleanupRenderTarget();
                g_Device.g_pSwapChain->ResizeBuffers(0, Rect.right, Rect.bottom, DXGI_FORMAT_UNKNOWN, 0);
                g_Device.CreateRenderTarget();
            }
        }

        Window.Pos = DestWindow.Pos = Vec2(static_cast<float>(Point.x), static_cast<float>(Point.y));
        Window.Size = DestWindow.Size = Vec2(static_cast<float>(Rect.right), static_cast<float>(Rect.bottom));

        return true;
    }

}