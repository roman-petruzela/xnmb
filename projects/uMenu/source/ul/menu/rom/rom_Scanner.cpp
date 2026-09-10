#include <ul/menu/rom/rom_Scanner.hpp>
#include <ul/menu/menu_Cache.hpp>
#include <ul/fs/fs_Stdio.hpp>
#include <ul/util/util_Json.hpp>
#include <algorithm>
#include <cctype>
#include <set>

namespace ul::menu::rom {

    namespace {

        std::string ToLower(const std::string &str) {
            std::string out = str;
            std::transform(out.begin(), out.end(), out.begin(), [](const unsigned char c) {
                return std::tolower(c);
            });
            return out;
        }

        // nro_argv is saved as: "<core_nro_path> \"<rom_path>\"" - extract the rom path back out
        // so a re-scan can tell which ROMs already have an entry.
        std::string ExtractRomPathFromArgv(const std::string &nro_argv) {
            const auto first_quote = nro_argv.find('"');
            if(first_quote == std::string::npos) {
                return "";
            }
            const auto last_quote = nro_argv.rfind('"');
            if(last_quote <= first_quote) {
                return "";
            }
            return nro_argv.substr(first_quote + 1, last_quote - first_quote - 1);
        }

        Entry FindOrCreateSystemFolder(const std::string &menu_base_path, const std::string &system_name) {
            const auto existing_entries = LoadEntries(menu_base_path);
            for(const auto &entry : existing_entries) {
                if(entry.Is<EntryType::Folder>() && (std::string(entry.folder_info.name) == system_name)) {
                    return entry;
                }
            }
            return CreateFolderEntry(menu_base_path, system_name);
        }

        std::set<std::string> ListKnownRomPaths(const std::string &folder_path) {
            std::set<std::string> known_rom_paths;
            for(const auto &entry : LoadEntries(folder_path)) {
                if(entry.Is<EntryType::Homebrew>()) {
                    const auto rom_path = ExtractRomPathFromArgv(entry.hb_info.nro_target.nro_argv);
                    if(!rom_path.empty()) {
                        known_rom_paths.insert(rom_path);
                    }
                }
            }
            return known_rom_paths;
        }

        bool MatchesAnyExtension(const std::string &file_name, const std::vector<std::string> &extensions) {
            const auto file_ext = ToLower(fs::GetExtension(file_name));
            for(const auto &ext : extensions) {
                if(file_ext == ToLower(ext)) {
                    return true;
                }
            }
            return false;
        }

    }

    bool LoadRomConfig(std::vector<RomSystemConfig> &out_systems) {
        out_systems.clear();

        util::JSON config_json;
        if(R_FAILED(util::LoadJSONFromFile(config_json, RomsConfigPath))) {
            return false;
        }

        if(!config_json.contains("systems") || !config_json["systems"].is_array()) {
            return false;
        }

        for(const auto &system_json : config_json["systems"]) {
            RomSystemConfig system {};
            system.name = system_json.value("name", "");
            system.core_nro_path = system_json.value("core_nro", "");
            if(system.name.empty() || system.core_nro_path.empty()) {
                continue;
            }

            if(system_json.contains("extensions") && system_json["extensions"].is_array()) {
                for(const auto &ext_json : system_json["extensions"]) {
                    system.extensions.push_back(ext_json.get<std::string>());
                }
            }

            if(system_json.contains("rom_dirs") && system_json["rom_dirs"].is_array()) {
                for(const auto &dir_json : system_json["rom_dirs"]) {
                    system.rom_dirs.push_back(dir_json.get<std::string>());
                }
            }

            out_systems.push_back(std::move(system));
        }

        return true;
    }

    RomScanResult ScanRoms(const std::string &menu_base_path) {
        RomScanResult result {};

        std::vector<RomSystemConfig> systems;
        if(!LoadRomConfig(systems)) {
            result.errors.push_back("Unable to load or parse " + std::string(RomsConfigPath));
            return result;
        }

        for(const auto &system : systems) {
            if(!fs::ExistsFile(system.core_nro_path)) {
                result.errors.push_back(system.name + ": core NRO not found at " + system.core_nro_path);
                continue;
            }

            // Ensures the core's NACP/icon are extracted into uLaunch's homebrew cache, so that
            // entries created below pick up a proper name/author/version/icon on first load.
            CacheHomebrewEntry(system.core_nro_path);

            const auto system_folder = FindOrCreateSystemFolder(menu_base_path, system.name);
            const auto folder_path = system_folder.GetFolderPath();
            auto known_rom_paths = ListKnownRomPaths(folder_path);

            for(const auto &rom_dir : system.rom_dirs) {
                if(!fs::ExistsDirectory(rom_dir)) {
                    result.errors.push_back(system.name + ": ROM directory not found: " + rom_dir);
                    continue;
                }

                UL_FS_FOR(rom_dir, file_name, file_path, is_dir, is_file, {
                    if(is_file && MatchesAnyExtension(file_name, system.extensions)) {
                        if(known_rom_paths.find(file_path) == known_rom_paths.end()) {
                            const auto display_name = fs::GetFileName(file_name);
                            const auto argv = system.core_nro_path + " \"" + file_path + "\"";

                            auto rom_entry = CreateHomebrewEntry(folder_path, system.core_nro_path, argv);
                            rom_entry.control.custom_name = true;
                            rom_entry.control.name = display_name;
                            rom_entry.Save();

                            known_rom_paths.insert(file_path);
                            result.new_entries_added++;
                        }
                    }
                });
            }

            result.systems_processed++;
        }

        return result;
    }

}
