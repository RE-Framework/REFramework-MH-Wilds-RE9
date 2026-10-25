// rev-t6pk7r-20261025 Config.h
// rev-d8e14f-20260829 Config.h
#pragma once
#include <string>
#include <vector>

struct ScriptEntry {
    std::string name;
    std::string path;
    std::string version;
    bool enabled = true;
};

struct ModEntry {
    std::string name;
    std::string path;
    std::string description;
    bool enabled = true;
};

class Config {
private:
    std::string m_configPath;

    Config() = default;

    void ParseLine(const std::string& line);
    void ScanForScripts();
    void ScanForMods();

public:
    bool m_enabled = true;
    bool m_showFPS = false;
    bool m_loadOnStartup = true;
    bool m_autoUpdate = true;
    bool m_vrEnabled = false;

    std::vector<ScriptEntry> m_loadedScripts;
    std::vector<ModEntry> m_installedMods;

    static Config& GetInstance();

    void Load(const std::string& path);
    void Save(const std::string& path);

    void ToggleScript(const std::string& name, bool enabled);
    void ToggleMod(const std::string& name, bool enabled);
};
