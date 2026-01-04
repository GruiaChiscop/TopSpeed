#include "GameManager.h"
#include "RaceManager.h"
#include "Vehicle.h"
#include "Track.h"
#include "../input/InputSystem.h"
#include "../input/RacingInput.h"
#include "../audio/AudioSystem.h"
#include "../ui/UISystem.h"
#include "../ui/RaceUI.h"
#include "../graphics/RenderSystem.h"
#include "../graphics/RaceRenderer.h"
#include <sstream>
#include <iomanip>
#include <chrono>

GameManager::GameManager() {
}

GameManager::~GameManager() {
    if (initialized_) {
        Shutdown();
    }
}

bool GameManager::Initialize(const GameConfig& config) {
    if (initialized_) {
        return true;
    }

    config_ = config;
    game_state_ = GameState::Loading;

    // Initialize all subsystems
    if (!InitializeSystems()) {
        Shutdown();
        return false;
    }

    // Setup initial game state
    game_state_ = GameState::MainMenu;
    initialized_ = true;
    return true;
}

void GameManager::Shutdown() {
    ShutdownSystems();
    game_state_ = GameState::Shutdown;
    initialized_ = false;
}

bool GameManager::InitializeSystems() {
    // Initialize rendering first (needed for UI)
    render_system_ = std::make_unique<RenderSystem>();
    RenderSystem::RenderConfig render_config;
    render_config.screen_width = config_.screen_width;
    render_config.screen_height = config_.screen_height;
    render_config.vsync_enabled = config_.vsync_enabled;
    render_config.anti_aliasing = config_.anti_aliasing;
    if (!render_system_->Initialize(render_config)) {
        return false;
    }

    // Initialize input systems
    input_system_ = std::make_unique<InputSystem>();
    if (!input_system_->Initialize()) {
        return false;
    }

    // Initialize UI system
    ui_system_ = std::make_unique<UISystem>();
    if (!ui_system_->Initialize(config_.screen_width, config_.screen_height)) {
        return false;
    }

    // Initialize audio system
    audio_system_ = std::make_unique<AudioSystem>();
    if (!audio_system_->Initialize()) {
        return false;
    }

    // Initialize race manager
    race_manager_ = std::make_unique<RaceManager>();
    if (!race_manager_->Initialize()) {
        return false;
    }

    // Initialize racing input (depends on input system and race manager)
    racing_input_ = std::make_unique<RacingInput>(input_system_.get(), race_manager_.get());
    if (!racing_input_->Initialize()) {
        return false;
    }

    // Initialize race UI (depends on race manager, vehicle, and UI system)
    race_ui_ = std::make_unique<RaceUI>(race_manager_.get(),
                                        race_manager_->GetPlayerVehicle(),
                                        racing_input_.get(),
                                        ui_system_.get());
    if (!race_ui_->Initialize()) {
        return false;
    }

    // Initialize race renderer (depends on race manager and render system)
    race_renderer_ = std::make_unique<RaceRenderer>(race_manager_.get(), render_system_.get());
    RaceRenderer::RaceRendererConfig renderer_config;
    renderer_config.render_track = true;
    renderer_config.render_vehicles = true;
    renderer_config.enable_particle_effects = true;
    renderer_config.show_vehicle_debug = config_.debug_mode;
    if (!race_renderer_->Initialize(renderer_config)) {
        return false;
    }

    return true;
}

void GameManager::ShutdownSystems() {
    // Shutdown in reverse order of initialization
    if (race_renderer_) race_renderer_->Shutdown();
    if (race_ui_) race_ui_->Shutdown();
    if (racing_input_) racing_input_->Shutdown();
    if (race_manager_) race_manager_->Shutdown();
    if (audio_system_) audio_system_->Shutdown();
    if (ui_system_) ui_system_->Shutdown();
    if (input_system_) input_system_->Shutdown();
    if (render_system_) render_system_->Shutdown();
}

void GameManager::Update(float deltaTime) {
    if (!initialized_) return;

    delta_time_ = deltaTime;
    total_time_ += deltaTime;
    frame_count_++;
    CalculateFPS(deltaTime);

    // Process input events
    ProcessEvents();

    // Update all systems based on game state
    switch (game_state_) {
        case GameState::Racing:
            UpdateSystems(deltaTime);
            break;

        case GameState::Paused:
            // Only update input to check for unpause
            if (input_system_) {
                input_system_->Update();
            }
            break;

        case GameState::MainMenu:
        case GameState::Loading:
        case GameState::RaceFinished:
            if (input_system_) {
                input_system_->Update();
            }
            break;

        default:
            break;
    }

    // Handle state transitions
    HandleGameStateTransitions();
}

void GameManager::UpdateSystems(float deltaTime) {
    // Update input
    if (input_system_) {
        input_system_->Update();
    }

    if (racing_input_) {
        racing_input_->Update(deltaTime);
    }

    // Update race logic
    if (race_manager_) {
        race_manager_->Update(deltaTime);
        
        // Check if race is finished
        auto player_info = race_manager_->GetRacerInfo(race_manager_->GetPlayerVehicle());
        if (player_info.race_finished) {
            SetGameState(GameState::RaceFinished);
        }
    }

    // Update audio based on vehicle state
    if (audio_system_ && race_manager_) {
        Vehicle* player_vehicle = race_manager_->GetPlayerVehicle();
        if (player_vehicle) {
            float engine_pitch = 0.5f + (player_vehicle->GetSpeed() / player_vehicle->GetMaxSpeed()) * 1.5f;
            audio_system_->SetEnginePitch(engine_pitch);
        }
    }

    // Update UI
    if (race_ui_) {
        race_ui_->Update(deltaTime);
    }

    if (ui_system_) {
        ui_system_->Render();
    }

    // Update camera for renderer
    if (race_renderer_ && race_manager_) {
        Vehicle* player_vehicle = race_manager_->GetPlayerVehicle();
        if (player_vehicle) {
            race_renderer_->UpdateCameraForPlayer(player_vehicle);
            race_renderer_->UpdateEffects(deltaTime);
        }
    }
}

void GameManager::Render() {
    if (!initialized_ || !render_system_ || !race_renderer_) return;

    switch (game_state_) {
        case GameState::Racing:
        case GameState::Paused:
            race_renderer_->BeginFrame();
            race_renderer_->RenderFrame();
            race_renderer_->EndFrame();
            break;

        case GameState::MainMenu:
        case GameState::Loading:
        case GameState::RaceFinished:
            render_system_->BeginFrame();
            render_system_->ClearScreen();
            render_system_->EndFrame();
            render_system_->Present();
            break;

        default:
            break;
    }

    // Debug info overlay (if enabled)
    if (config_.show_debug_info && game_state_ == GameState::Racing) {
        // Debug info would be rendered here
    }

    if (config_.show_fps) {
        // FPS display would be rendered here
    }
}

void GameManager::ProcessEvents() {
    if (!input_system_) return;

    // Check for quit
    if (input_system_->IsKeyPressed(InputSystem::KeyCode::Escape)) {
        if (game_state_ == GameState::Racing) {
            PauseGame();
        } else if (game_state_ == GameState::Paused) {
            ResumeGame();
        } else {
            QuitGame();
        }
    }

    // Check for pause toggle
    if (input_system_->IsKeyPressed(InputSystem::KeyCode::P) && 
        game_state_ == GameState::Racing) {
        PauseGame();
    }
}

void GameManager::SetGameState(GameState state) {
    if (game_state_ == state) return;

    GameState old_state = game_state_;
    game_state_ = state;

    // Handle state-specific transitions
    switch (state) {
        case GameState::Racing:
            if (audio_system_) {
                audio_system_->ResumeMusic();
                audio_system_->PlaySound("race_start");
            }
            break;

        case GameState::Paused:
            if (audio_system_) {
                audio_system_->PauseMusic();
            }
            if (ui_system_) {
                ui_system_->ShowPauseMenu();
            }
            break;

        case GameState::RaceFinished:
            if (audio_system_) {
                audio_system_->PauseMusic();
                audio_system_->PlaySound("race_complete");
            }
            break;

        default:
            break;
    }
}

void GameManager::StartRace() {
    if (!race_manager_) return;

    // Setup race
    RaceManager::RaceConfig race_config;
    race_config.num_laps = config_.num_laps;
    race_config.num_racers = config_.num_racers;
    race_config.difficulty = static_cast<RaceManager::DifficultyMode>(config_.difficulty);
    race_config.enable_collisions = config_.enable_collisions;
    race_config.enable_damage = config_.enable_damage;

    race_manager_->SetRaceConfig(race_config);
    race_manager_->StartRace();

    SetGameState(GameState::Racing);
}

void GameManager::PauseGame() {
    if (game_state_ == GameState::Racing) {
        SetGameState(GameState::Paused);
    }
}

void GameManager::ResumeGame() {
    if (game_state_ == GameState::Paused) {
        SetGameState(GameState::Racing);
    }
}

void GameManager::EndRace() {
    if (race_manager_) {
        race_manager_->EndRace();
    }
    SetGameState(GameState::MainMenu);
}

void GameManager::RestartRace() {
    EndRace();
    StartRace();
}

void GameManager::QuitGame() {
    game_state_ = GameState::Shutdown;
}

const char* GameManager::GetGameStateString() const {
    switch (game_state_) {
        case GameState::Uninitialized:
            return "Uninitialized";
        case GameState::MainMenu:
            return "Main Menu";
        case GameState::Loading:
            return "Loading";
        case GameState::Racing:
            return "Racing";
        case GameState::Paused:
            return "Paused";
        case GameState::RaceFinished:
            return "Race Finished";
        case GameState::Shutdown:
            return "Shutdown";
        default:
            return "Unknown";
    }
}

void GameManager::CalculateFPS(float deltaTime) {
    fps_timer_ += deltaTime;
    if (fps_timer_ >= 1.0f) {
        fps_ = frame_count_ / fps_timer_;
        frame_count_ = 0;
        fps_timer_ = 0.0f;
    }
}

void GameManager::HandleGameStateTransitions() {
    // Additional state transition logic can go here
}

void GameManager::UpdateInputBindings() {
    // Update input binding based on manual transmission setting
    if (racing_input_) {
        racing_input_->SetAutomatic(!config_.manual_transmission);
    }
}

std::string GameManager::GetDebugInfo() const {
    std::stringstream ss;
    ss << "GameManager Debug Info\n";
    ss << "- State: " << GetGameStateString() << "\n";
    ss << "- Initialized: " << (initialized_ ? "Yes" : "No") << "\n";
    ss << "- Frame Count: " << frame_count_ << "\n";
    ss << "- FPS: " << std::fixed << std::setprecision(1) << fps_ << "\n";
    ss << "- Total Time: " << std::setprecision(2) << total_time_ << "s\n";
    ss << "- Delta Time: " << std::setprecision(3) << delta_time_ << "s\n";
    ss << "- Screen: " << config_.screen_width << "x" << config_.screen_height << "\n";
    ss << "- Audio: " << (config_.audio_enabled ? "Enabled" : "Disabled") << "\n";
    ss << "- Master Volume: " << std::setprecision(1) << config_.master_volume * 100.0f << "%\n";
    ss << "- Difficulty: ";
    switch (config_.difficulty) {
        case DifficultyMode::Easy:
            ss << "Easy";
            break;
        case DifficultyMode::Normal:
            ss << "Normal";
            break;
        case DifficultyMode::Hard:
            ss << "Hard";
            break;
        case DifficultyMode::Expert:
            ss << "Expert";
            break;
    }
    ss << "\n";
    ss << "- Num Laps: " << config_.num_laps << "\n";
    ss << "- Num Racers: " << config_.num_racers << "\n";
    ss << "- Manual Transmission: " << (config_.manual_transmission ? "Yes" : "No") << "\n";
    return ss.str();
}

std::string GameManager::GetSystemsStatus() const {
    std::stringstream ss;
    ss << "System Status\n";
    ss << "- RenderSystem: " << (render_system_ && render_system_->GetLoadedTextureCount() > 0 ? "OK" : "Not Ready") << "\n";
    ss << "- InputSystem: " << (input_system_ ? "OK" : "Not Ready") << "\n";
    ss << "- RacingInput: " << (racing_input_ ? "OK" : "Not Ready") << "\n";
    ss << "- AudioSystem: " << (audio_system_ ? "OK" : "Not Ready") << "\n";
    ss << "- UISystem: " << (ui_system_ ? "OK" : "Not Ready") << "\n";
    ss << "- RaceUI: " << (race_ui_ ? "OK" : "Not Ready") << "\n";
    ss << "- RaceManager: " << (race_manager_ ? "OK" : "Not Ready") << "\n";
    ss << "- RaceRenderer: " << (race_renderer_ ? "OK" : "Not Ready") << "\n";
    return ss.str();
}
