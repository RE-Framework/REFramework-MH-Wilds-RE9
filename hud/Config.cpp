// rev-t6pk7r-20261025 Config.cpp
// rev-d8e14f-20260829 Config.cpp
#include "Config.h"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

Config& Config::GetInstance() {
    static Config instance;
    return instance;
}

void Config::Load(const std::string& path) {
    m_configPath = path;
    m_enabled = true;
    m_showFPS = false;
    m_loadOnStartup = true;
    m_autoUpdate = true;
    m_vrEnabled = false;

    fs::path configPath(path);
    if (fs::exists(configPath)) {
        std::ifstream file(path);
        std::string line;
        while (std::getline(file, line)) {
            ParseLine(line);
        }
    }

    ScanForScripts();
    ScanForMods();
}

void Config::Save(const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) return;

    file << "[reframework]\n";
    file << "enabled=" << (m_enabled ? "1" : "0") << "\n";
    file << "show_fps=" << (m_showFPS ? "1" : "0") << "\n";
    file << "load_on_startup=" << (m_loadOnStartup ? "1" : "0") << "\n";
    file << "auto_update=" << (m_autoUpdate ? "1" : "0") << "\n";
    file << "vr_enabled=" << (m_vrEnabled ? "1" : "0") << "\n";

    file << "\n[scripts]\n";
    for (const auto& s : m_loadedScripts) {
        file << s.name << "=" << (s.enabled ? "1" : "0") << "\n";
    }

    file << "\n[mods]\n";
    for (const auto& m : m_installedMods) {
        file << m.name << "=" << (m.enabled ? "1" : "0") << "\n";
    }
}

void Config::ParseLine(const std::string& line) {
    if (line.empty() || line[0] == '[' || line[0] == '#') return;

    size_t eq = line.find('=');
    if (eq == std::string::npos) return;

    std::string key = line.substr(0, eq);
    std::string val = line.substr(eq + 1);

    if (key == "enabled") m_enabled = (val == "1");
    else if (key == "show_fps") m_showFPS = (val == "1");
    else if (key == "load_on_startup") m_loadOnStartup = (val == "1");
    else if (key == "auto_update") m_autoUpdate = (val == "1");
    else if (key == "vr_enabled") m_vrEnabled = (val == "1");
}

void Config::ScanForScripts() {
    m_loadedScripts.clear();

    fs::path scriptDir = "reframework/autorun";
    if (!fs::exists(scriptDir)) {
        fs::create_directories(scriptDir);
        return;
    }

    for (const auto& entry : fs::directory_iterator(scriptDir)) {
        if (entry.path().extension() == ".lua") {
            ScriptEntry s;
            s.name = entry.path().stem().string();
            s.path = entry.path().string();
            s.enabled = true;
            s.version = "1.0";
            m_loadedScripts.push_back(s);
        }
    }
}

void Config::ScanForMods() {
    m_installedMods.clear();

    fs::path modDir = "natives";
    if (!fs::exists(modDir)) return;

    for (const auto& entry : fs::directory_iterator(modDir)) {
        if (entry.is_directory()) {
            ModEntry m;
            m.name = entry.path().filename().string();
            m.enabled = true;
            m.path = entry.path().string();
            m_installedMods.push_back(m);
        }
    }
}

void Config::ToggleScript(const std::string& name, bool enabled) {
    for (auto& s : m_loadedScripts) {
        if (s.name == name) {
            s.enabled = enabled;
            break;
        }
    }
}

void Config::ToggleMod(const std::string& name, bool enabled) {
    for (auto& m : m_installedMods) {
        if (m.name == name) {
            m.enabled = enabled;
            break;
        }
    }
}
