// rev-t6pk7r-20261025 API.cpp
// rev-d8e14f-20260829 API.cpp
#include "API.h"
#include "Hooks.h"
#include <sstream>

API& API::GetInstance() {
    static API instance;
    return instance;
}

void API::Initialize() {
    m_luaState = luaL_newstate();
    luaL_openlibs(m_luaState);

    RegisterGlobalNamespace();
    RegisterRENamespace();
    RegisterSDKNamespace();
    RegisterImGuiNamespace();

    LoadAutorunScripts();
}

void API::RegisterGlobalNamespace() {
    lua_newtable(m_luaState);

    lua_pushcfunction(m_luaState, Lua_GetGameVersion);
    lua_setfield(m_luaState, -2, "get_game_version");

    lua_pushcfunction(m_luaState, Lua_GetFrameworkVersion);
    lua_setfield(m_luaState, -2, "get_framework_version");

    lua_setglobal(m_luaState, "reframework");
}

void API::RegisterRENamespace() {
    lua_getglobal(m_luaState, "reframework");

    lua_newtable(m_luaState);

    lua_newtable(m_luaState);
    lua_pushcfunction(m_luaState, Lua_RE_Camera_GetActive);
    lua_setfield(m_luaState, -2, "get_active_camera");
    lua_pushcfunction(m_luaState, Lua_RE_Camera_SetFOV);
    lua_setfield(m_luaState, -2, "set_fov");
    lua_setfield(m_luaState, -2, "camera");

    lua_newtable(m_luaState);
    lua_pushcfunction(m_luaState, Lua_RE_GameObject_Find);
    lua_setfield(m_luaState, -2, "find");
    lua_pushcfunction(m_luaState, Lua_RE_GameObject_Spawn);
    lua_setfield(m_luaState, -2, "spawn");
    lua_setfield(m_luaState, -2, "game_object");

    lua_newtable(m_luaState);
    lua_pushcfunction(m_luaState, Lua_RE_Scene_SetTimeScale);
    lua_setfield(m_luaState, -2, "set_time_scale");
    lua_setfield(m_luaState, -2, "scene");

    lua_newtable(m_luaState);
    lua_pushcfunction(m_luaState, Lua_RE_Audio_SetVolume);
    lua_setfield(m_luaState, -2, "set_volume");
    lua_setfield(m_luaState, -2, "audio");

    lua_setglobal(m_luaState, "re");
    lua_pop(m_luaState, 1);
}

void API::RegisterSDKNamespace() {
    lua_newtable(m_luaState);

    lua_pushcfunction(m_luaState, Lua_SDK_GetNativeField);
    lua_setfield(m_luaState, -2, "get_native_field");

    lua_pushcfunction(m_luaState, Lua_SDK_SetNativeField);
    lua_setfield(m_luaState, -2, "set_native_field");

    lua_pushcfunction(m_luaState, Lua_SDK_CallNative);
    lua_setfield(m_luaState, -2, "call_native");

    lua_setglobal(m_luaState, "sdk");
}

void API::RegisterImGuiNamespace() {
    lua_newtable(m_luaState);

    lua_pushcfunction(m_luaState, Lua_ImGui_Begin);
    lua_setfield(m_luaState, -2, "begin");
    lua_pushcfunction(m_luaState, Lua_ImGui_End);
    lua_setfield(m_luaState, -2, "end");
    lua_pushcfunction(m_luaState, Lua_ImGui_Button);
    lua_setfield(m_luaState, -2, "button");
    lua_pushcfunction(m_luaState, Lua_ImGui_SliderFloat);
    lua_setfield(m_luaState, -2, "slider_float");
    lua_pushcfunction(m_luaState, Lua_ImGui_Checkbox);
    lua_setfield(m_luaState, -2, "checkbox");
    lua_pushcfunction(m_luaState, Lua_ImGui_Text);
    lua_setfield(m_luaState, -2, "text");

    lua_setglobal(m_luaState, "imgui");
}

void API::LoadAutorunScripts() {
    namespace fs = std::filesystem;
    fs::path autorunDir = "reframework/autorun";

    if (!fs::exists(autorunDir)) {
        fs::create_directories(autorunDir);
        return;
    }

    for (const auto& entry : fs::directory_iterator(autorunDir)) {
        if (entry.path().extension() == ".lua") {
            if (luaL_dofile(m_luaState, entry.path().string().c_str()) != LUA_OK) {
                const char* err = lua_tostring(m_luaState, -1);
                OutputDebugStringA("[REFramework] Lua error: ");
                OutputDebugStringA(err);
                OutputDebugStringA("\n");
            }
        }
    }
}

void API::ExecuteString(const std::string& code) {
    if (luaL_dostring(m_luaState, code.c_str()) != LUA_OK) {
        const char* err = lua_tostring(m_luaState, -1);
        m_lastError = err ? std::string(err) : "unknown error";
    }
}

void API::OnPresent() {
    lua_getglobal(m_luaState, "OnPresent");
    if (lua_isfunction(m_luaState, -1)) {
        if (lua_pcall(m_luaState, 0, 0, 0) != LUA_OK) {
            const char* err = lua_tostring(m_luaState, -1);
            OutputDebugStringA("[REFramework] OnPresent error: ");
            OutputDebugStringA(err);
            OutputDebugStringA("\n");
        }
    }
    lua_pop(m_luaState, 1);
}

void API::OnPreGuiDraw() {
    lua_getglobal(m_luaState, "OnPreGuiDraw");
    if (lua_isfunction(m_luaState, -1)) {
        lua_pcall(m_luaState, 0, 0, 0);
    }
    lua_pop(m_luaState, 1);
}
