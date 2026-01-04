#pragma once

#include <string>
#include <cstdint>

namespace TopSpeed {

/**
 * Game state enumeration
 * Represents where in the game flow we currently are
 */
enum class GameState {
    // Menu states
    MainMenu,           // Main menu screen
    MenuSettings,       // Settings/options menu
    MenuAudio,          // Audio settings
    MenuControls,       // Control binding menu
    MenuAbout,          // About/credits
    
    // Race states
    RaceLoading,        // Loading race
    RaceCountdown,      // 3-2-1-GO countdown
    RaceActive,         // Race in progress
    RacePaused,         // Race paused
    RaceFinished,       // Race finished, showing results
    
    // Multiplayer states
    MultiplayerLobby,   // Waiting for players
    MultiplayerSync,    // Syncing with other players
    
    // Special states
    Loading,            // General loading state
    Error,              // Error occurred
    ShuttingDown        // Engine shutting down
};

/**
 * Convert game state to string (for debugging)
 */
inline const char* GameStateToString(GameState state) {
    switch (state) {
        case GameState::MainMenu:          return "MainMenu";
        case GameState::MenuSettings:      return "MenuSettings";
        case GameState::MenuAudio:         return "MenuAudio";
        case GameState::MenuControls:      return "MenuControls";
        case GameState::MenuAbout:         return "MenuAbout";
        case GameState::RaceLoading:       return "RaceLoading";
        case GameState::RaceCountdown:     return "RaceCountdown";
        case GameState::RaceActive:        return "RaceActive";
        case GameState::RacePaused:        return "RacePaused";
        case GameState::RaceFinished:      return "RaceFinished";
        case GameState::MultiplayerLobby:  return "MultiplayerLobby";
        case GameState::MultiplayerSync:   return "MultiplayerSync";
        case GameState::Loading:           return "Loading";
        case GameState::Error:             return "Error";
        case GameState::ShuttingDown:      return "ShuttingDown";
        default:                           return "Unknown";
    }
}

/**
 * Game configuration
 * Persistent settings for the game
 */
struct GameConfig {
    // Audio settings
    float master_volume = 1.0f;          // Master volume (0.0-1.0)
    float music_volume = 0.8f;           // Music volume
    float sfx_volume = 0.9f;             // Sound effects volume
    float ui_volume = 0.7f;              // UI sounds volume
    float dialogue_volume = 1.0f;        // Dialogue volume
    bool audio_enabled = true;           // Enable/disable audio
    
    // Accessibility
    bool screen_reader_enabled = true;   // Enable screen reader
    bool tts_enabled = true;             // Enable text-to-speech
    float speech_rate = 1.0f;            // Speech rate (0.5-2.0)
    float speech_volume = 1.0f;          // Speech volume (0.0-1.0)
    
    // Input settings
    float controller_dead_zone = 0.15f;  // Joystick dead zone
    bool controller_rumble = true;       // Enable controller vibration
    bool invert_steering = false;        // Invert steering axis
    float steering_sensitivity = 1.0f;   // Steering sensitivity
    
    // Game settings
    uint8_t difficulty = 1;              // 0=Easy, 1=Normal, 2=Hard
    bool multiplayer_enabled = true;     // Enable network play
    std::string player_name = "Player";  // Player name
    uint16_t network_port = 9999;        // Network port
    
    // Display (placeholder for future rendering)
    uint32_t display_width = 1280;
    uint32_t display_height = 720;
    bool fullscreen = false;
    uint8_t target_fps = 60;
    
    /**
     * Save configuration to file
     */
    bool Save(const std::string& filename) const;
    
    /**
     * Load configuration from file
     */
    bool Load(const std::string& filename);
    
    /**
     * Reset to default values
     */
    void Reset();
    
    /**
     * Get debug info
     */
    std::string GetDebugInfo() const;
};

} // namespace TopSpeed
