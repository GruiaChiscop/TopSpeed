#pragma once

#include "GameState.h"
#include "GameClock.h"
#include "../audio/AudioEngine.h"
#include "../audio/SoundMixer.h"
#include "../input/InputManager.h"
#include "../network/NetworkIntegration.h"
#include "../accessibility/ScreenReaderManager.h"

namespace TopSpeed {

/**
 * Main game engine - coordinates all subsystems
 * 
 * Architecture inspired by NVGT's game loop:
 * - Fixed 60 Hz update rate (deterministic)
 * - Variable-rate rendering
 * - Unified subsystem coordination
 * - Event-driven input processing
 * - Accessibility integrated throughout
 * 
 * Usage:
 *   GameEngine engine;
 *   engine.Initialize();
 *   
 *   while (engine.IsRunning()) {
 *       engine.Update();
 *   }
 *   
 *   engine.Shutdown();
 */
class GameEngine {
public:
    /**
     * Game engine events for callbacks
     */
    enum class EngineEvent {
        Initialized,
        ShutdownStarted,
        ShutdownComplete,
        GameStateChanged,
        PauseToggled,
        SpeedChanged
    };
    
    /**
     * Engine event callback
     */
    using EngineEventCallback = std::function<void(EngineEvent event)>;
    
    GameEngine();
    ~GameEngine();
    
    // Initialization & Shutdown
    /**
     * Initialize all engine subsystems
     * @return True if successful
     */
    bool Initialize();
    
    /**
     * Shutdown all subsystems gracefully
     */
    void Shutdown();
    
    /**
     * Check if engine is initialized and running
     */
    bool IsInitialized() const { return initialized_; }
    bool IsRunning() const { return running_; }
    
    /**
     * Request engine shutdown
     */
    void RequestShutdown() { running_ = false; }
    
    // Main loop - call this once per iteration
    /**
     * Update engine (call once per frame)
     * Handles fixed-timestep updates and variable rendering
     * @return True if update was performed
     */
    bool Update();
    
    // Time control
    /**
     * Get fixed timestep (1/60 second)
     */
    static float GetFixedTimestep() { return FIXED_TIMESTEP; }
    
    /**
     * Get actual delta time for this frame
     */
    float GetDeltaTime() const { return game_clock_.GetDeltaTime(); }
    
    /**
     * Get total elapsed time since startup
     */
    double GetTotalTime() const { return game_clock_.GetTotalTime(); }
    
    /**
     * Get current frame number
     */
    uint64_t GetFrameNumber() const { return frame_number_; }
    
    /**
     * Get current FPS
     */
    float GetFrameRate() const { return game_clock_.GetFrameRate(); }
    
    /**
     * Set target frame rate for rendering (0 = unlimited)
     */
    void SetTargetFrameRate(float fps) { target_frame_rate_ = fps; }
    float GetTargetFrameRate() const { return target_frame_rate_; }
    
    // Game state
    /**
     * Get current game state
     */
    GameState GetGameState() const { return game_state_; }
    
    /**
     * Change game state
     */
    void SetGameState(GameState new_state);
    
    /**
     * Check if game is paused
     */
    bool IsPaused() const { return paused_; }
    
    /**
     * Toggle pause
     */
    void TogglePause();
    
    /**
     * Set game speed multiplier (1.0 = normal)
     */
    void SetGameSpeed(float speed);
    float GetGameSpeed() const { return game_speed_; }
    
    // Subsystem access
    /**
     * Get audio engine
     */
    AudioEngine& GetAudioEngine() { return audio_engine_; }
    const AudioEngine& GetAudioEngine() const { return audio_engine_; }
    
    /**
     * Get mixer manager
     */
    MixerManager& GetMixerManager() { return mixer_manager_; }
    
    /**
     * Get input manager
     */
    InputManager& GetInputManager() { return input_manager_; }
    const InputManager& GetInputManager() const { return input_manager_; }
    
    /**
     * Get network integration
     */
    NetworkIntegration& GetNetworkManager() { return network_manager_; }
    
    /**
     * Get accessibility system
     */
    ScreenReaderManager& GetAccessibility() { return accessibility_; }
    
    // Events
    /**
     * Register callback for engine events
     */
    void SetEventCallback(EngineEventCallback callback) { event_callback_ = callback; }
    
    // Configuration
    /**
     * Get engine configuration
     */
    const GameConfig& GetConfig() const { return config_; }
    
    /**
     * Update engine configuration
     */
    void UpdateConfig(const GameConfig& config);
    
    // Debug
    /**
     * Get engine debug info
     */
    std::string GetDebugInfo() const;
    
    /**
     * Enable/disable debug display
     */
    void SetDebugMode(bool enabled) { debug_mode_ = enabled; }
    bool IsDebugMode() const { return debug_mode_; }

private:
    // Fixed timestep parameters
    static constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;  // 60 Hz update rate
    static constexpr float MAX_FRAME_TIME = 0.1f;          // Max 100ms frame time
    
    // Engine state
    bool initialized_ = false;
    bool running_ = false;
    bool paused_ = false;
    bool debug_mode_ = false;
    
    // Timing
    GameClock game_clock_;
    float accumulated_time_ = 0.0f;
    float game_speed_ = 1.0f;
    float target_frame_rate_ = 60.0f;
    uint64_t frame_number_ = 0;
    
    // Game state
    GameState game_state_ = GameState::MainMenu;
    GameState prev_game_state_ = GameState::MainMenu;
    
    // Subsystems
    AudioEngine audio_engine_;
    MixerManager mixer_manager_;
    InputManager input_manager_;
    NetworkIntegration network_manager_;
    ScreenReaderManager accessibility_;
    GameConfig config_;
    
    // Callbacks
    EngineEventCallback event_callback_;
    
    // Internal methods
    void InitializeSubsystems();
    void ShutdownSubsystems();
    void HandleInput();
    void UpdateGame(float delta_time);
    void UpdateAudio();
    void UpdateNetwork();
    void CheckFrameRate();
    void FireEvent(EngineEvent event);
};

} // namespace TopSpeed
