// rev-t6pk7r-20261025 Hooks.cpp
// rev-d8e14f-20260829 Hooks.cpp
#include "Hooks.h"
#include "Overlay.h"
#include <d3d11.h>
#include <d3d12.h>
#include <MinHook.h>

#pragma comment(lib, "d3d11.lib")

Hooks& Hooks::GetInstance() {
    static Hooks instance;
    return instance;
}

typedef HRESULT(WINAPI* PresentD3D11)(IDXGISwapChain*, UINT, UINT);
typedef HRESULT(WINAPI* PresentD3D12)(IDXGISwapChain*, UINT, UINT);
typedef LRESULT(CALLBACK* WndProcDef)(HWND, UINT, WPARAM, LPARAM);

static PresentD3D11 origPresent11 = nullptr;
static WndProcDef origWndProc = nullptr;
static HWND gameHwnd = nullptr;
static bool gameWindowReady = false;

HRESULT WINAPI HookedPresent11(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) {
    Overlay::GetInstance().Render(swapChain);
    return origPresent11(swapChain, syncInterval, flags);
}

LRESULT CALLBACK HookedWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (Overlay::GetInstance().HandleMessage(hWnd, msg, wParam, lParam)) {
        return 0;
    }
    return CallWindowProc(origWndProc, hWnd, msg, wParam, lParam);
}

bool Hooks::Initialize() {
    if (MH_Initialize() != MH_OK) {
        OutputDebugStringA("[REFramework] MH_Initialize failed");
        return false;
    }

    IDXGISwapChain* dummySwap = CreateDummySwapChain();
    if (!dummySwap) {
        OutputDebugStringA("[REFramework] Failed to create dummy swap chain for vtable hooking");
        return false;
    }

    void** vtable = *(void***)dummySwap;
    void* presentAddr = vtable[8];

    if (MH_CreateHook(presentAddr, &HookedPresent11, (LPVOID*)&origPresent11) != MH_OK) {
        dummySwap->Release();
        return false;
    }

    if (MH_EnableHook(presentAddr) != MH_OK) {
        dummySwap->Release();
        return false;
    }

    dummySwap->Release();

    HookREEngineInternals();

    OutputDebugStringA("[REFramework] Hooks initialized successfully");
    return true;
}

void Hooks::HookREEngineInternals() {
    HMODULE hModule = GetModuleHandleA(nullptr);

    auto scanPattern = [](HMODULE mod, const char* pattern, const char* mask) -> void* {
        MODULEINFO info;
        GetModuleInformation(GetCurrentProcess(), mod, &info, sizeof(info));

        auto base = (uint8_t*)info.lpBaseOfDll;
        auto size = info.SizeOfImage;
        size_t patLen = strlen(mask);

        for (size_t i = 0; i <= size - patLen; i++) {
            bool found = true;
            for (size_t j = 0; j < patLen; j++) {
                if (mask[j] == 'x' && base[i + j] != (uint8_t)pattern[j]) {
                    found = false;
                    break;
                }
            }
            if (found) return base + i;
        }
        return nullptr;
    };

    void* renderTargetSet = scanPattern(hModule,
        "\x48\x8B\x01\x48\x85\xC0\x74\x05", "xxxxxxxx");

    if (renderTargetSet) {
        MH_CreateHook(renderTargetSet, &Hooks::HookedRenderTargetSet, nullptr);
        MH_EnableHook(renderTargetSet);
    }
}

void Hooks::WaitForGameWindow() {
    while (!gameHwnd) {
        gameHwnd = FindWindowA("RE_ENGINE_WINDOW", nullptr);
        if (!gameHwnd) {
            gameHwnd = FindWindowA(nullptr, "MonsterHunterWilds");
        }
        if (!gameHwnd) {
            gameHwnd = FindWindowA(nullptr, "RESIDENT EVIL");
        }
        if (!gameHwnd) {
            Sleep(500);
        }
    }

    origWndProc = (WndProcDef)SetWindowLongPtr(gameHwnd, GWLP_WNDPROC, (LONG_PTR)HookedWndProc);
    gameWindowReady = true;
    OutputDebugStringA("[REFramework] Game window found and hooked");
}

void Hooks::Shutdown() {
    if (origWndProc && gameHwnd) {
        SetWindowLongPtr(gameHwnd, GWLP_WNDPROC, (LONG_PTR)origWndProc);
    }
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
}

IDXGISwapChain* Hooks::CreateDummySwapChain() {
    DXGI_SWAP_CHAIN_DESC desc = {};
    desc.BufferCount = 2;
    desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferDesc.Width = 1;
    desc.BufferDesc.Height = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow = GetDesktopWindow();
    desc.SampleDesc.Count = 1;
    desc.Windowed = TRUE;
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    IDXGISwapChain* swap = nullptr;
    D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDware, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION,
        &desc, &swap, nullptr, nullptr, nullptr
    );

    return swap;
}

HWND Hooks::GetGameWindow() {
    return gameHwnd;
}
