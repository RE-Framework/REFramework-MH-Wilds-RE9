// rev-t6pk7r-20261025 Overlay.h
// rev-d8e14f-20260829 Overlay.h
#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <imgui.h>
#include <string>
#include <vector>
#include <atomic>

enum class Tab {
    Main,
    Camera,
    Scripts,
    Mods,
    Config,
    About
};

class Overlay {
private:
    HWND m_hwnd = nullptr;
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;
    IDXGISwapChain* m_swapChain = nullptr;
    ID3D11RenderTargetView* m_rtv = nullptr;

    bool m_isVisible = false;
    Tab m_selectedTab = Tab::Main;
    ImFont* m_defaultFont = nullptr;
    std::atomic<bool> m_running = true;

    Overlay() = default;

    bool GetD3D11DeviceFromSwapChain(HWND hwnd);
    void InitImGui();
    void LoadFont();
    void DrawMainWindow();
    void DrawMainTab();
    void DrawCameraTab();
    void DrawScriptsTab();
    void DrawModsTab();
    void DrawConfigTab();
    void DrawAboutTab();

public:
    static Overlay& GetInstance();

    void Initialize(HWND hwnd);
    void Render(IDXGISwapChain* swapChain);
    bool HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void Run();
    void Shutdown();

    bool IsVisible() const { return m_isVisible; }
    void SetVisible(bool visible) { m_isVisible = visible; }
};
