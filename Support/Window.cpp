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

    // Create Swap Chain
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

    // Create RTV Heap
    D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
    descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    descHeapDesc.NumDescriptors = FrameCount;
    descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    descHeapDesc.NodeMask = 0;
    if (FAILED(DXContext::Get().GetDevice()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&m_rtvDescHeap))))
    {
        return false;
    }

    // Create Handles to View
    auto firstHandle = m_rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
    auto handleIncrement = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    for (size_t i = 0; i < FrameCount; ++i)
    {
        m_rtvHandles[i] = firstHandle;
        m_rtvHandles[i].ptr += handleIncrement * i;
    }

    // Get Buffers
    if (!GetBuffers())
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
    ReleaseBuffers();

    m_rtvDescHeap.Release();

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

void DXWindow::Resize()
{
    ReleaseBuffers();

    RECT clientRect;
    if (GetClientRect(m_window, &clientRect))
    {
        m_width = clientRect.right - clientRect.left; 
        m_height = clientRect.bottom - clientRect.top;

        m_swapChain->ResizeBuffers(
            GetFrameCount(),
            m_width,
            m_height,
            DXGI_FORMAT_UNKNOWN,
            DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
        );
        m_shouldResize = false;
    }

    GetBuffers();
}

void DXWindow::SetFullscreen(bool enabled)
{
    // Update Window Styling for Fullscreen
    DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    DWORD exStyle = WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW;
    if (enabled)
    {
        style = WS_POPUP | WS_VISIBLE;
        exStyle = WS_EX_APPWINDOW;
    }

    SetWindowLong(m_window, GWL_STYLE, style);
    SetWindowLong(m_window, GWL_EXSTYLE, exStyle);

    // Adjust Window Size
    if (enabled)
    {
        HMONITOR monitor = MonitorFromWindow(m_window, MONITOR_DEFAULTTONEAREST);
        MONITORINFO monitorInfo{};
        monitorInfo.cbSize = sizeof(monitorInfo);
        if (GetMonitorInfo(monitor, &monitorInfo))
        {
            SetWindowPos(
                m_window,
                nullptr,
                monitorInfo.rcMonitor.left,
                monitorInfo.rcMonitor.top,
                monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                SWP_NOZORDER
            );
        }
    }
    else
    {
        ShowWindow(m_window, SW_MAXIMIZE);
    }

    m_isFullscreen = enabled;
}

void DXWindow::BeginFrame(ID3D12GraphicsCommandList10* cmdList)
{
    m_currentBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

    D3D12_RESOURCE_BARRIER barr;
    barr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barr.Transition.pResource = m_buffers[m_currentBufferIndex];
    barr.Transition.Subresource = 0;
    barr.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barr.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    cmdList->ResourceBarrier(1, &barr);

    float clearColour[] = { .2f, .4f, .2f, 1.f };
    cmdList->ClearRenderTargetView(m_rtvHandles[m_currentBufferIndex], clearColour, 0, nullptr);
    cmdList->OMSetRenderTargets(1, &m_rtvHandles[m_currentBufferIndex], false, nullptr);
}

void DXWindow::EndFrame(ID3D12GraphicsCommandList10* cmdList)
{
    D3D12_RESOURCE_BARRIER barr;
    barr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barr.Transition.pResource = m_buffers[m_currentBufferIndex];
    barr.Transition.Subresource = 0;
    barr.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barr.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

    cmdList->ResourceBarrier(1, &barr);
}

bool DXWindow::GetBuffers()
{
    for (UINT i = 0; i < FrameCount; ++i)
    {
        if (FAILED(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_buffers[i]))))
        {
            return false;
        }

        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
        rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;
        rtvDesc.Texture2D.PlaneSlice = 0;
        DXContext::Get().GetDevice()->CreateRenderTargetView(
            m_buffers[i],
            &rtvDesc,
            m_rtvHandles[i]
        );
    }

    return true;
}

void DXWindow::ReleaseBuffers()
{
    for (size_t i = 0; i < FrameCount; ++i)
    {
        m_buffers[i].Release();
    }
}

LRESULT DXWindow::OnWindowMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
        if (wParam == VK_F11)
        {
            Get().SetFullscreen(!Get().IsFullscreen());
        }
        break;

    case WM_SIZE:
        if (lParam && (HIWORD(lParam) != Get().m_height || LOWORD(lParam) != Get().m_width))
        {
            Get().m_shouldResize = true;
        }
        break;

    case WM_CLOSE:
        Get().m_shouldClose = true;
        return 0;
    }

    return DefWindowProc(wnd, msg, wParam, lParam);
}
