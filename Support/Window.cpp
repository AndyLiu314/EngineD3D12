#include "Window.h"

bool DXWindow::Init()
{
    // Window Class
    WNDCLASSEXW wcex{};
    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = &DXWindow::OnWindowMessage;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = GetModuleHandle(nullptr);
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = nullptr;
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"EngineD3D12WndCls";
    wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    m_wndClass = RegisterClassEx(&wcex);
    if (m_wndClass == 0)
    {
        return false;
    }

    // Window Spawn Location
    POINT pos{ 0, 0 };
    GetCursorPos(&pos);
    HMONITOR monitor = MonitorFromPoint(pos, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO monitorInfo{};
    monitorInfo.cbSize = sizeof(monitorInfo);
    GetMonitorInfo(monitor, &monitorInfo);

    // Creating Window
    m_window = CreateWindowEx(
        WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW,
        (LPCWSTR)m_wndClass,
        L"EngineD3D12",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        monitorInfo.rcWork.left + 100,
        monitorInfo.rcWork.top + 100,
        1920,
        1080,
        nullptr,
        nullptr,
        wcex.hInstance,
        nullptr);
    if (m_window == nullptr)
    {
        return false;
    }

    // Describe Swap Chain
    DXGI_SWAP_CHAIN_DESC1 swapDesc{};
    swapDesc.Width = 1920;
    swapDesc.Height = 1080;
    swapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.Stereo = false;
    swapDesc.SampleDesc.Count = 1;
    swapDesc.SampleDesc.Quality = 0;
    swapDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.BufferCount = GetFrameCount();
    swapDesc.Scaling = DXGI_SCALING_STRETCH;
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapFullDesc{};
    swapFullDesc.Windowed = true;

    // Swap Chain
    auto& factory = DXContext::Get().GetFactory();
    ComPointer<IDXGISwapChain1> sc1;
    factory->CreateSwapChainForHwnd(
        DXContext::Get().GetCommandQueue(),
        m_window,
        &swapDesc,
        &swapFullDesc,
        nullptr,
        &sc1
    );
    if (!sc1.QueryInterface(m_swapChain))
    {
        return false;
    }

    return true;
}

void DXWindow::Update()
{
    MSG msg;
    while (PeekMessage(&msg, m_window, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void DXWindow::Present()
{
    m_swapChain->Present(1, 0);
}

void DXWindow::Shutdown()
{
    m_swapChain.Release();

    if (m_window)
    {
        DestroyWindow(m_window);
    }

    if (m_wndClass)
    {
        UnregisterClass((LPCWSTR)m_wndClass, GetModuleHandle(nullptr));
    }
}

LRESULT DXWindow::OnWindowMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
        Get().m_shouldClose = true;
        return 0;
    }

    return DefWindowProc(wnd, msg, wParam, lParam);
}
