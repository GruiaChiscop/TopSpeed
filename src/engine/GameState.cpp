#include "GameState.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>

namespace TopSpeed {

bool GameConfig::Save(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to save config to " << filename << std::endl;
        return false;
    }
    
    // Write magic number for validation
    uint32_t magic = 0x54535046;  // "TSPF" (TopSpeed Profile)
    file.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    
    // Version
    uint32_t version = 1;
    file.write(reinterpret_cast<const char*>(&version), sizeof(version));
    
    // Audio settings
    file.write(reinterpret_cast<const char*>(&master_volume), sizeof(master_volume));
    file.write(reinterpret_cast<const char*>(&music_volume), sizeof(music_volume));
    file.write(reinterpret_cast<const char*>(&sfx_volume), sizeof(sfx_volume));
    file.write(reinterpret_cast<const char*>(&ui_volume), sizeof(ui_volume));
    file.write(reinterpret_cast<const char*>(&dialogue_volume), sizeof(dialogue_volume));
    file.write(reinterpret_cast<const char*>(&audio_enabled), sizeof(audio_enabled));
    
    // Accessibility settings
    file.write(reinterpret_cast<const char*>(&screen_reader_enabled), sizeof(screen_reader_enabled));
    file.write(reinterpret_cast<const char*>(&tts_enabled), sizeof(tts_enabled));
    file.write(reinterpret_cast<const char*>(&speech_rate), sizeof(speech_rate));
    file.write(reinterpret_cast<const char*>(&speech_volume), sizeof(speech_volume));
    
    // Input settings
    file.write(reinterpret_cast<const char*>(&controller_dead_zone), sizeof(controller_dead_zone));
    file.write(reinterpret_cast<const char*>(&controller_rumble), sizeof(controller_rumble));
    file.write(reinterpret_cast<const char*>(&invert_steering), sizeof(invert_steering));
    file.write(reinterpret_cast<const char*>(&steering_sensitivity), sizeof(steering_sensitivity));
    
    // Game settings
    file.write(reinterpret_cast<const char*>(&difficulty), sizeof(difficulty));
    file.write(reinterpret_cast<const char*>(&multiplayer_enabled), sizeof(multiplayer_enabled));
    file.write(reinterpret_cast<const char*>(&network_port), sizeof(network_port));
    
    // Write player name (with length prefix)
    uint32_t name_len = player_name.length();
    file.write(reinterpret_cast<const char*>(&name_len), sizeof(name_len));
    file.write(player_name.data(), name_len);
    
    // Display settings
    file.write(reinterpret_cast<const char*>(&display_width), sizeof(display_width));
    file.write(reinterpret_cast<const char*>(&display_height), sizeof(display_height));
    file.write(reinterpret_cast<const char*>(&fullscreen), sizeof(fullscreen));
    file.write(reinterpret_cast<const char*>(&target_fps), sizeof(target_fps));
    
    if (file.fail()) {
        std::cerr << "Error writing config file" << std::endl;
        return false;
    }
    
    std::cout << "Configuration saved to " << filename << std::endl;
    return true;
}

bool GameConfig::Load(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to load config from " << filename << std::endl;
        return false;
    }
    
    // Read and validate magic number
    uint32_t magic;
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    if (magic != 0x54535046) {
        std::cerr << "Invalid config file format" << std::endl;
        return false;
    }
    
    // Check version
    uint32_t version;
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    if (version != 1) {
        std::cerr << "Unsupported config version: " << version << std::endl;
        return false;
    }
    
    // Audio settings
    file.read(reinterpret_cast<char*>(&master_volume), sizeof(master_volume));
    file.read(reinterpret_cast<char*>(&music_volume), sizeof(music_volume));
    file.read(reinterpret_cast<char*>(&sfx_volume), sizeof(sfx_volume));
    file.read(reinterpret_cast<char*>(&ui_volume), sizeof(ui_volume));
    file.read(reinterpret_cast<char*>(&dialogue_volume), sizeof(dialogue_volume));
    file.read(reinterpret_cast<char*>(&audio_enabled), sizeof(audio_enabled));
    
    // Accessibility settings
    file.read(reinterpret_cast<char*>(&screen_reader_enabled), sizeof(screen_reader_enabled));
    file.read(reinterpret_cast<char*>(&tts_enabled), sizeof(tts_enabled));
    file.read(reinterpret_cast<char*>(&speech_rate), sizeof(speech_rate));
    file.read(reinterpret_cast<char*>(&speech_volume), sizeof(speech_volume));
    
    // Input settings
    file.read(reinterpret_cast<char*>(&controller_dead_zone), sizeof(controller_dead_zone));
    file.read(reinterpret_cast<char*>(&controller_rumble), sizeof(controller_rumble));
    file.read(reinterpret_cast<char*>(&invert_steering), sizeof(invert_steering));
    file.read(reinterpret_cast<char*>(&steering_sensitivity), sizeof(steering_sensitivity));
    
    // Game settings
    file.read(reinterpret_cast<char*>(&difficulty), sizeof(difficulty));
    file.read(reinterpret_cast<char*>(&multiplayer_enabled), sizeof(multiplayer_enabled));
    file.read(reinterpret_cast<char*>(&network_port), sizeof(network_port));
    
    // Read player name
    uint32_t name_len;
    file.read(reinterpret_cast<char*>(&name_len), sizeof(name_len));
    if (name_len > 0 && name_len < 256) {
        player_name.resize(name_len);
        file.read(&player_name[0], name_len);
    }
    
    // Display settings
    file.read(reinterpret_cast<char*>(&display_width), sizeof(display_width));
    file.read(reinterpret_cast<char*>(&display_height), sizeof(display_height));
    file.read(reinterpret_cast<char*>(&fullscreen), sizeof(fullscreen));
    file.read(reinterpret_cast<char*>(&target_fps), sizeof(target_fps));
    
    if (file.fail()) {
        std::cerr << "Error reading config file" << std::endl;
        return false;
    }
    
    std::cout << "Configuration loaded from " << filename << std::endl;
    return true;
}

void GameConfig::Reset() {
    master_volume = 1.0f;
    music_volume = 0.8f;
    sfx_volume = 0.9f;
    ui_volume = 0.7f;
    dialogue_volume = 1.0f;
    audio_enabled = true;
    
    screen_reader_enabled = true;
    tts_enabled = true;
    speech_rate = 1.0f;
    speech_volume = 1.0f;
    
    controller_dead_zone = 0.15f;
    controller_rumble = true;
    invert_steering = false;
    steering_sensitivity = 1.0f;
    
    difficulty = 1;
    multiplayer_enabled = true;
    player_name = "Player";
    network_port = 9999;
    
    display_width = 1280;
    display_height = 720;
    fullscreen = false;
    target_fps = 60;
}

std::string GameConfig::GetDebugInfo() const {
    std::ostringstream oss;
    oss << "=== Game Configuration ===\n\n";
    
    oss << "Audio:\n";
    oss << "  Master: " << (master_volume * 100) << "%\n";
    oss << "  Music: " << (music_volume * 100) << "%\n";
    oss << "  SFX: " << (sfx_volume * 100) << "%\n";
    oss << "  UI: " << (ui_volume * 100) << "%\n";
    oss << "  Dialogue: " << (dialogue_volume * 100) << "%\n";
    oss << "  Enabled: " << (audio_enabled ? "yes" : "no") << "\n\n";
    
    oss << "Accessibility:\n";
    oss << "  Screen Reader: " << (screen_reader_enabled ? "yes" : "no") << "\n";
    oss << "  TTS: " << (tts_enabled ? "yes" : "no") << "\n";
    oss << "  Speech Rate: " << speech_rate << "x\n";
    oss << "  Speech Volume: " << (speech_volume * 100) << "%\n\n";
    
    oss << "Input:\n";
    oss << "  Dead Zone: " << dead_zone_ << "\n";
    oss << "  Controller Rumble: " << (controller_rumble ? "yes" : "no") << "\n";
    oss << "  Invert Steering: " << (invert_steering ? "yes" : "no") << "\n";
    oss << "  Steering Sensitivity: " << steering_sensitivity << "x\n\n";
    
    oss << "Game:\n";
    oss << "  Player Name: " << player_name << "\n";
    oss << "  Difficulty: " << static_cast<int>(difficulty) << "\n";
    oss << "  Multiplayer: " << (multiplayer_enabled ? "yes" : "no") << "\n";
    oss << "  Network Port: " << network_port << "\n\n";
    
    oss << "Display:\n";
    oss << "  Resolution: " << display_width << "x" << display_height << "\n";
    oss << "  Fullscreen: " << (fullscreen ? "yes" : "no") << "\n";
    oss << "  Target FPS: " << static_cast<int>(target_fps);
    
    return oss.str();
}

} // namespace TopSpeed
