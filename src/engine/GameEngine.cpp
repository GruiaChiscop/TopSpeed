#include "GameEngine.h"
#include <iostream>
#include <sstream>

namespace TopSpeed {

GameEngine::GameEngine()
    : initialized_(false), running_(false), paused_(false), debug_mode_(false),
      accumulated_time_(0.0f), game_speed_(1.0f), target_frame_rate_(60.0f),
      frame_number_(0), game_state_(GameState::MainMenu) {}

GameEngine::~GameEngine() {
    if (initialized_) {
        Shutdown();
    }
}

bool GameEngine::Initialize() {
    if (initialized_) {
        std::cerr << "Engine already initialized" << std::endl;
        return false;
    }
    
    std::cout << "\n=== Initializing TopSpeed Game Engine ===" << std::endl;
    
    // Initialize subsystems in order
    InitializeSubsystems();
    
    initialized_ = true;
    running_ = true;
    
    std::cout << "\n✓ Game Engine initialized successfully" << std::endl;
    std::cout << "  Fixed Timestep: " << (1.0f/60.0f) << "s (60 Hz)" << std::endl;
    std::cout << "  Target Frame Rate: " << target_frame_rate_ << " FPS" << std::endl;
    
    // Fire initialization event
    FireEvent(EngineEvent::Initialized);
    
    return true;
}

void GameEngine::Shutdown() {
    if (!initialized_) {
        return;
    }
    
    std::cout << "\n=== Shutting down TopSpeed Game Engine ===" << std::endl;
    
    running_ = false;
    FireEvent(EngineEvent::ShutdownStarted);
    
    ShutdownSubsystems();
    
    initialized_ = false;
    
    std::cout << "\u2713 Game Engine shutdown complete" << std::endl;
    FireEvent(EngineEvent::ShutdownComplete);
}

bool GameEngine::Update() {
    if (!running_ || !initialized_) {
        return false;
    }
    
    // Update clock
    game_clock_.Update();
    float delta_time = game_clock_.GetDeltaTime();
    
    // Accumulate time for fixed timestep
    accumulated_time_ += delta_time * game_speed_;
    
    // Handle input first
    HandleInput();
    
    // Fixed timestep updates
    bool performed_update = false;
    while (accumulated_time_ >= FIXED_TIMESTEP) {
        // Update game logic
        if (!paused_) {
            UpdateGame(FIXED_TIMESTEP);
        }
        
        accumulated_time_ -= FIXED_TIMESTEP;
        performed_update = true;
    }
    
    // Update continuous systems (audio, network)
    UpdateAudio();
    UpdateNetwork();
    
    // Check frame rate limiting
    CheckFrameRate();
    
    frame_number_++;
    
    return performed_update;
}

void GameEngine::SetGameState(GameState new_state) {
    if (game_state_ != new_state) {
        GameState old_state = game_state_;
        game_state_ = new_state;
        
        std::cout << "Game State: " << GameStateToString(old_state)
                  << " -> " << GameStateToString(new_state) << std::endl;
        
        // Fire event and announcement
        accessibility_.AnnounceEvent(std::string("Game state: ") + GameStateToString(new_state));
        FireEvent(EngineEvent::GameStateChanged);
    }
}

void GameEngine::TogglePause() {
    paused_ = !paused_;
    
    if (paused_) {
        accessibility_.Speak("Game paused.");
    } else {
        accessibility_.Speak("Game resumed.");
    }
    
    FireEvent(EngineEvent::PauseToggled);
}

void GameEngine::SetGameSpeed(float speed) {
    game_speed_ = std::max(0.0f, speed);
    
    if (speed == 1.0f) {
        accessibility_.AnnounceEvent("Speed: normal");
    } else if (speed < 1.0f) {
        accessibility_.AnnounceEvent(std::string("Speed: slow (x") + std::to_string(game_speed_) + ")");
    } else {
        accessibility_.AnnounceEvent(std::string("Speed: fast (x") + std::to_string(game_speed_) + ")");
    }
    
    FireEvent(EngineEvent::SpeedChanged);
}

void GameEngine::UpdateConfig(const GameConfig& config) {
    config_ = config;
    
    // Apply audio config
    mixer_manager_.SetMasterVolume(config_.master_volume);
    
    // Apply accessibility config
    if (config_.screen_reader_enabled) {
        accessibility_.SetEnabled(true);
    }
}

std::string GameEngine::GetDebugInfo() const {
    std::ostringstream oss;
    oss << "\n=== TopSpeed Engine Debug Info ===\n\n";
    oss << "Status:\n";
    oss << "  Initialized: " << (initialized_ ? "yes" : "no") << "\n";
    oss << "  Running: " << (running_ ? "yes" : "no") << "\n";
    oss << "  Paused: " << (paused_ ? "yes" : "no") << "\n";
    oss << "  Game Speed: " << game_speed_ << "x\n";
    oss << "  Current State: " << GameStateToString(game_state_) << "\n\n";
    
    oss << game_clock_.GetDebugInfo() << "\n\n";
    
    oss << "Subsystems:\n";
    oss << "  Audio: " << (audio_engine_.IsInitialized() ? "initialized" : "not initialized") << "\n";
    oss << "  Mixers: " << mixer_manager_.GetMixerCount() << " active\n";
    oss << "  Input: ready\n";
    oss << "  Network: ready\n";
    oss << "  Accessibility: ready\n";
    
    return oss.str();
}

// Private methods

void GameEngine::InitializeSubsystems() {
    std::cout << "\nInitializing subsystems..." << std::endl;
    
    // Audio engine
    if (!audio_engine_.Initialize()) {
        std::cerr << "Failed to initialize audio engine" << std::endl;
        return;
    }
    
    // Mixer manager (create standard mixers)
    mixer_manager_.CreateMixer(MixerManager::MIXER_MUSIC);
    mixer_manager_.CreateMixer(MixerManager::MIXER_SFX);
    mixer_manager_.CreateMixer(MixerManager::MIXER_UI);
    mixer_manager_.CreateMixer(MixerManager::MIXER_DIALOGUE);
    std::cout << "  ✓ Audio system initialized with 4 mixers" << std::endl;
    
    // Input manager
    if (!input_manager_.Initialize()) {
        std::cerr << "Failed to initialize input manager" << std::endl;
        return;
    }
    std::cout << "  ✓ Input manager initialized" << std::endl;
    
    // Network integration
    if (!network_manager_.Initialize()) {
        std::cerr << "Failed to initialize network" << std::endl;
        return;
    }
    std::cout << "  ✓ Network manager initialized" << std::endl;
    
    // Accessibility
    if (!accessibility_.Initialize()) {
        std::cerr << "Warning: Accessibility not available" << std::endl;
    } else {
        std::cout << "  ✓ Screen reader initialized" << std::endl;
    }
    
    // Game clock
    game_clock_.Reset();
    std::cout << "  ✓ Game clock initialized" << std::endl;
}

void GameEngine::ShutdownSubsystems() {
    std::cout << "Shutting down subsystems..." << std::endl;
    
    accessibility_.Shutdown();
    network_manager_.Shutdown();
    input_manager_.Shutdown();
    audio_engine_.Shutdown();
    mixer_manager_.ClearMixers();
    
    std::cout << "  ✓ All subsystems shut down" << std::endl;
}

void GameEngine::HandleInput() {
    // Update input state
    input_manager_.Update();
    
    // Handle escape key (pause)
    if (input_manager_.IsKeyPressed("pause")) {
        TogglePause();
    }
}

void GameEngine::UpdateGame(float delta_time) {
    // This is where game logic goes
    // For now, just a placeholder
}

void GameEngine::UpdateAudio() {
    // Update audio engine
    audio_engine_.Update();
    
    // Update mixer manager
    mixer_manager_.Update(game_clock_.GetDeltaTime());
}

void GameEngine::UpdateNetwork() {
    // Update network state
    network_manager_.Update();
}

void GameEngine::CheckFrameRate() {
    if (target_frame_rate_ > 0.0f) {
        float frame_time = 1.0f / target_frame_rate_;
        float elapsed = game_clock_.GetDeltaTime();
        float remaining = frame_time - elapsed;
        
        // Simple frame limiting (would normally use sleep)
        if (remaining > 0.0f) {
            // In a real application, would call sleep here
            // For now, just track it
        }
    }
}

void GameEngine::FireEvent(EngineEvent event) {
    if (event_callback_) {
        event_callback_(event);
    }
}

} // namespace TopSpeed
