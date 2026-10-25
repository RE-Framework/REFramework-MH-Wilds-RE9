// rev-t6pk7r-20261025 Overlay.cpp
// rev-d8e14f-20260829 Overlay.cpp
#include "Overlay.h"
#include "API.h"
#include "Config.h"
#include <d3d11.h>
#include <d3d11on12.h>

Overlay& Overlay::GetInstance() {
    static Overlay instance;
    return instance;
}

void Overlay::Initialize(HWND hwnd) {
    m_hwnd = hwnd;
    m_isVisible = false;
    m_selectedTab = Tab::Main;

    if (!GetD3D11DeviceFromSwapChain(hwnd)) {
        OutputDebugStringA("[REFramework] Failed to get D3D11 device");
        return;
    }

    InitImGui();
    LoadFont();

    OutputDebugStringA("[REFramework] Overlay initialized");
}

bool Overlay::GetD3D11DeviceFromSwapChain(HWND hwnd) {
    return true;
}

void Overlay::InitImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = "reframework/imgui.ini";

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 2.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
}

void Overlay::LoadFont() {
    ImGuiIO& io = ImGui::GetIO();
    m_defaultFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 16.0f);
}

void Overlay::Render(IDXGISwapChain* swapChain) {
    static bool firstFrame = true;
    if (firstFrame) {
        firstFrame = false;
        return;
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (m_isVisible) {
        DrawMainWindow();
    }

    API::GetInstance().OnPreGuiDraw();

    ImGui::EndFrame();
    ImGui::Render();
}

void Overlay::DrawMainWindow() {
    ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("REFramework", &m_isVisible, ImGuiWindowFlags_MenuBar)) {
        ImGui::End();
        return;
    }

    if (ImGui::BeginMenuBar()) {
        if (ImGui::MenuItem("Main")) m_selectedTab = Tab::Main;
        if (ImGui::MenuItem("Camera")) m_selectedTab = Tab::Camera;
        if (ImGui::MenuItem("Scripts")) m_selectedTab = Tab::Scripts;
        if (ImGui::MenuItem("Mods")) m_selectedTab = Tab::Mods;
        if (ImGui::MenuItem("Config")) m_selectedTab = Tab::Config;
        if (ImGui::MenuItem("About")) m_selectedTab = Tab::About;
        ImGui::EndMenuBar();
    }

    switch (m_selectedTab) {
        case Tab::Main: DrawMainTab(); break;
        case Tab::Camera: DrawCameraTab(); break;
        case Tab::Scripts: DrawScriptsTab(); break;
        case Tab::Mods: DrawModsTab(); break;
        case Tab::Config: DrawConfigTab(); break;
        case Tab::About: DrawAboutTab(); break;
    }

    ImGui::End();
}

void Overlay::DrawMainTab() {
    ImGui::Text("REFramework v1.5.9.2");
    ImGui::Separator();
    ImGui::Text("RE Engine Modding Framework");
    ImGui::Text("Author: praydog");
    ImGui::Separator();

    ImGui::Checkbox("Enable REFramework", &Config::GetInstance().m_enabled);
    ImGui::Checkbox("Show FPS", &Config::GetInstance().m_showFPS);

    static float fps = 60.0f;
    ImGui::SliderFloat("Target FPS", &fps, 30.0f, 144.0f);
}

void Overlay::DrawCameraTab() {
    ImGui::Text("Camera Controls");
    ImGui::Separator();

    static bool freecamEnabled = false;
    ImGui::Checkbox("Enable Freecam", &freecamEnabled);

    if (freecamEnabled) {
        static float speed = 1.0f;
        ImGui::SliderFloat("Speed", &speed, 0.1f, 10.0f);

        static float fov = 75.0f;
        ImGui::SliderFloat("FOV", &fov, 30.0f, 120.0f);

        static float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
        ImGui::SliderFloat("Pos X", &posX, -1000.0f, 1000.0f);
        ImGui::SliderFloat("Pos Y", &posY, -1000.0f, 1000.0f);
        ImGui::SliderFloat("Pos Z", &posZ, -1000.0f, 1000.0f);

        if (ImGui::Button("Reset Position")) {
            posX = posY = posZ = 0.0f;
        }
    }

    ImGui::Separator();
    ImGui::Text("Controls: WASD to move, Mouse to look, Shift for boost");
}

void Overlay::DrawScriptsTab() {
    ImGui::Text("Loaded Scripts");
    ImGui::Separator();

    auto& scripts = Config::GetInstance().m_loadedScripts;
    for (size_t i = 0; i < scripts.size(); i++) {
        ImGui::PushID((int)i);
        ImGui::Checkbox("##enabled", &scripts[i].enabled);
        ImGui::SameLine();
        ImGui::Text("%s", scripts[i].name.c_str());
        ImGui::SameLine();
        ImGui::TextDisabled("(v%s)", scripts[i].version.c_str());
        ImGui::PopID();
    }

    ImGui::Separator();
    if (ImGui::Button("Reload All Scripts")) {
        API::GetInstance().ReloadScripts();
    }
}

void Overlay::DrawModsTab() {
    ImGui::Text("Installed Mods");
    ImGui::Separator();

    auto& mods = Config::GetInstance().m_installedMods;
    for (size_t i = 0; i < mods.size(); i++) {
        ImGui::PushID((int)i);
        ImGui::Checkbox("##enabled", &mods[i].enabled);
        ImGui::SameLine();
        ImGui::Text("%s", mods[i].name.c_str());
        if (!mods[i].description.empty()) {
            ImGui::SameLine();
            ImGui::TextDisabled("- %s", mods[i].description.c_str());
        }
        ImGui::PopID();
    }
}

void Overlay::DrawConfigTab() {
    ImGui::Text("Configuration");
    ImGui::Separator();

    ImGui::Checkbox("Load on Startup", &Config::GetInstance().m_loadOnStartup);
    ImGui::Checkbox("Auto-update", &Config::GetInstance().m_autoUpdate);
    ImGui::Checkbox("Enable VR Mode", &Config::GetInstance().m_vrEnabled);

    ImGui::Separator();
    if (ImGui::Button("Save Config")) {
        Config::GetInstance().Save("reframework/config.ini");
    }
}

void Overlay::DrawAboutTab() {
    ImGui::Text("REFramework");
    ImGui::Text("Version: 1.5.9.2 (nightly)");
    ImGui::Separator();
    ImGui::Text("Created by: praydog");
    ImGui::Text("License: MIT");
    ImGui::Separator();
    ImGui::Text("Supports: Monster Hunter Wilds, RE9, RE4, RE8, RE2, RE3, RE7, DD2, SF6");
    ImGui::Separator();
    ImGui::Text("Press INSERT to toggle this overlay");
}

bool Overlay::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN && wParam == VK_INSERT) {
        m_isVisible = !m_isVisible;
        return true;
    }

    if (m_isVisible) {
        ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
        return true;
    }

    return false;
}

void Overlay::Run() {
    while (m_running) {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(16);
    }
}

void Overlay::Shutdown() {
    m_running = false;
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}
