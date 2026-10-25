// rev-t6pk7r-20261025 Hooks.h
// rev-d8e14f-20260829 Hooks.h
#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <cstdint>

class Hooks {
private:
    Hooks() = default;

    static IDXGISwapChain* CreateDummySwapChain();
    static void HookREEngineInternals();

public:
    static Hooks& GetInstance();

    static bool Initialize();
    static void Shutdown();

    static void WaitForGameWindow();
    static HWND GetGameWindow();

    static HRESULT WINAPI HookedRenderTargetSet();
};
