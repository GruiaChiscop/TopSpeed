#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>

// Forward declarations
class RaceManager;
class Vehicle;
class Track;
class InputSystem;
class RacingInput;
class AudioSystem;
class UISystem;
class RaceUI;
class RenderSystem;
class RaceRenderer;

/**
 * @class GameManager
 * @brief Central manager integrating all game systems
 * 
 * Handles initialization, updating, and rendering of all game subsystems.
 * Provides the main game loop interface and system lifecycle management.
 */
class GameManager {
public:
    enum class GameState {
        Uninitialized,
        MainMenu,
        Loading,
        Racing,
        Paused,
        RaceFinished,
        Shutdown
    };

    enum class DifficultyMode {
        Easy,      // Slower AI, forgiving physics
        Normal,    // Standard AI and physics
        Hard,      // Aggressive AI, realistic physics
        Expert     // Very aggressive AI, hardcore physics
    };

    struct GameConfig {
        // Graphics
        int screen_width = 1920;
        int screen_height = 1080;
        bool vsync_enabled = true;
        bool anti_aliasing = true;

        // Audio
        bool audio_enabled = true;
        float master_volume = 0.8f;
        float music_volume = 0.6f;
        float sfx_volume = 0.8f;

        // Gameplay
        DifficultyMode difficulty = DifficultyMode::Normal;
        int num_laps = 3;
        int num_racers = 4;
        bool enable_collisions = true;
        bool enable_damage = true;

        // Input
        bool use_keyboard = true;
        bool use_gamepad = true;
        bool manual_transmission = false;

        // Debug
        bool debug_mode = false;
        bool show_fps = false;
        bool show_debug_info = false;
    };

    GameManager();
    ~GameManager();

    // Lifecycle
    bool Initialize(const GameConfig& config);
    void Shutdown();
    bool IsInitialized() const { return initialized_; }

    // Game loop
    void Update(float deltaTime);
    void Render();
    void ProcessEvents();
    bool IsRunning() const { return game_state_ != GameState::Shutdown; }

    // State management
    void SetGameState(GameState state);
    GameState GetGameState() const { return game_state_; }
    const char* GetGameStateString() const;

    // Game control
    void StartRace();
    void PauseGame();
    void ResumeGame();
    void EndRace();
    void RestartRace();
    void QuitGame();

    // Configuration
    void SetConfig(const GameConfig& config) { config_ = config; }
    GameConfig GetConfig() const { return config_; }

    // System access
    RaceManager* GetRaceManager() { return race_manager_.get(); }
    InputSystem* GetInputSystem() { return input_system_.get(); }
    RacingInput* GetRacingInput() { return racing_input_.get(); }
    AudioSystem* GetAudioSystem() { return audio_system_.get(); }
    UISystem* GetUISystem() { return ui_system_.get(); }
    RaceUI* GetRaceUI() { return race_ui_.get(); }
    RenderSystem* GetRenderSystem() { return render_system_.get(); }
    RaceRenderer* GetRaceRenderer() { return race_renderer_.get(); }

    // Timing
    float GetDeltaTime() const { return delta_time_; }
    float GetTotalTime() const { return total_time_; }
    int GetFrameCount() const { return frame_count_; }
    float GetFPS() const { return fps_; }

    // Debug
    std::string GetDebugInfo() const;
    std::string GetSystemsStatus() const;

private:
    // Game state
    GameState game_state_ = GameState::Uninitialized;
    GameConfig config_;
    bool initialized_ = false;

    // Timing
    float delta_time_ = 0.0f;
    float total_time_ = 0.0f;
    int frame_count_ = 0;
    float fps_ = 0.0f;
    float fps_timer_ = 0.0f;

    // Subsystems (unique pointers for automatic cleanup)
    std::unique_ptr<RaceManager> race_manager_;
    std::unique_ptr<InputSystem> input_system_;
    std::unique_ptr<RacingInput> racing_input_;
    std::unique_ptr<AudioSystem> audio_system_;
    std::unique_ptr<UISystem> ui_system_;
    std::unique_ptr<RaceUI> race_ui_;
    std::unique_ptr<RenderSystem> render_system_;
    std::unique_ptr<RaceRenderer> race_renderer_;

    // Helper methods
    bool InitializeSystems();
    void ShutdownSystems();
    void UpdateSystems(float deltaTime);
    void RenderSystems();
    void CalculateFPS(float deltaTime);
    void HandleGameStateTransitions();
    void UpdateInputBindings();
};
