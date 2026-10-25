// rev-t6pk7r-20261025 API.h
// rev-d8e14f-20260829 API.h
#pragma once
#include <lua.hpp>
#include <string>
#include <vector>
#include <filesystem>

class API {
private:
    lua_State* m_luaState = nullptr;
    std::string m_lastError;

    API() = default;

    void RegisterGlobalNamespace();
    void RegisterRENamespace();
    void RegisterSDKNamespace();
    void RegisterImGuiNamespace();
    void LoadAutorunScripts();

    static int Lua_GetGameVersion(lua_State* L);
    static int Lua_GetFrameworkVersion(lua_State* L);
    static int Lua_RE_Camera_GetActive(lua_State* L);
    static int Lua_RE_Camera_SetFOV(lua_State* L);
    static int Lua_RE_GameObject_Find(lua_State* L);
    static int Lua_RE_GameObject_Spawn(lua_State* L);
    static int Lua_RE_Scene_SetTimeScale(lua_State* L);
    static int Lua_RE_Audio_SetVolume(lua_State* L);
    static int Lua_SDK_GetNativeField(lua_State* L);
    static int Lua_SDK_SetNativeField(lua_State* L);
    static int Lua_SDK_CallNative(lua_State* L);
    static int Lua_ImGui_Begin(lua_State* L);
    static int Lua_ImGui_End(lua_State* L);
    static int Lua_ImGui_Button(lua_State* L);
    static int Lua_ImGui_SliderFloat(lua_State* L);
    static int Lua_ImGui_Checkbox(lua_State* L);
    static int Lua_ImGui_Text(lua_State* L);

public:
    static API& GetInstance();

    void Initialize();
    void ReloadScripts() { LoadAutorunScripts(); }
    void ExecuteString(const std::string& code);
    void OnPresent();
    void OnPreGuiDraw();

    lua_State* GetLuaState() { return m_luaState; }
    const std::string& GetLastError() const { return m_lastError; }
};
