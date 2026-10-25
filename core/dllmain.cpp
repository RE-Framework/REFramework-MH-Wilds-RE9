// rev-t6pk7r-20261025 dllmain.cpp
// rev-d8e14f-20260829 dllmain.cpp
#include <Windows.h>
#include "Hooks.h"
#include "Overlay.h"
#include "API.h"
#include "Config.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
    switch (dwReason) {
        case DLL_PROCESS_ATTACH: {
            DisableThreadLibraryCalls(hModule);

            Config::GetInstance().Load("reframework/config.ini");

            if (!Hooks::Initialize()) {
                OutputDebugStringA("[REFramework] Failed to initialize hooks");
                return FALSE;
            }

            API::GetInstance().Initialize();

            CreateThread(nullptr, 0, [](LPVOID param) -> DWORD {
                Hooks::WaitForGameWindow();

                Overlay::GetInstance().Initialize((HWND)Hooks::GetGameWindow());
                Overlay::GetInstance().Run();

                return 0;
            }, nullptr, 0, nullptr);

            break;
        }
        case DLL_PROCESS_DETACH: {
            Hooks::Shutdown();
            Overlay::GetInstance().Shutdown();
            break;
        }
    }
    return TRUE;
}
