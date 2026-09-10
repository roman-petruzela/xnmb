
#pragma once
#include <ul/menu/menu_Entries.hpp>
#include <vector>
#include <string>

namespace ul::menu::rom {

    // sdmc:/ulaunch/roms_config.json - see roms_config.example.json at the repo root for the schema.
    constexpr const char RomsConfigPath[] = "sdmc:/ulaunch/roms_config.json";

    struct RomSystemConfig {
        std::string name;
        std::string core_nro_path;
        std::vector<std::string> extensions;
        std::vector<std::string> rom_dirs;
    };

    struct RomScanResult {
        u32 systems_processed = 0;
        u32 new_entries_added = 0;
        std::vector<std::string> errors;

        inline bool HasErrors() const {
            return !this->errors.empty();
        }
    };

    // Returns false (with no systems loaded) if the config file does not exist or fails to parse.
    bool LoadRomConfig(std::vector<RomSystemConfig> &out_systems);

    // For every configured system, ensures a folder entry exists under menu_base_path, then
    // creates one Homebrew entry per ROM found (skipping ROMs that already have an entry),
    // launching RetroArch's core NRO for that system with the ROM path as content argument.
    RomScanResult ScanRoms(const std::string &menu_base_path);

}
