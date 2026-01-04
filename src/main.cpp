#include "game/GameManager.h"
#include <iostream>
#include <chrono>
#include <iomanip>

/**
 * @file main.cpp
 * @brief TopSpeed Racing Game - Main Entry Point
 * 
 * Initializes the game engine and runs the main game loop with
 * fixed time steps for consistent physics and gameplay updates.
 */

class GameApp {
public:
    GameApp() : game_manager_(nullptr) {}

    bool Initialize() {
        // Create game manager
        game_manager_ = std::make_unique<GameManager>();

        // Configure game
        GameManager::GameConfig config;
        config.screen_width = 1920;
        config.screen_height = 1080;
        config.vsync_enabled = true;
        config.anti_aliasing = true;
        config.audio_enabled = true;
        config.master_volume = 0.8f;
        config.music_volume = 0.6f;
        config.sfx_volume = 0.8f;
        config.difficulty = GameManager::DifficultyMode::Normal;
        config.num_laps = 3;
        config.num_racers = 4;
        config.enable_collisions = true;
        config.enable_damage = true;
        config.use_keyboard = true;
        config.use_gamepad = true;
        config.manual_transmission = false;  // Auto transmission by default
        config.debug_mode = false;
        config.show_fps = true;  // Show FPS counter
        config.show_debug_info = false;

        // Initialize game
        if (!game_manager_->Initialize(config)) {
            std::cerr << "Failed to initialize game!" << std::endl;
            return false;
        }

        std::cout << "Game initialized successfully!" << std::endl;
        std::cout << game_manager_->GetSystemsStatus() << std::endl;

        return true;
    }

    void Run() {
        // Start race immediately for demo
        game_manager_->StartRace();

        // Timing constants
        const float TARGET_FPS = 60.0f;
        const float FIXED_TIMESTEP = 1.0f / TARGET_FPS;  // 16.67ms per frame
        const float MAX_FRAME_TIME = 0.1f;  // Max 100ms per frame

        // Game loop variables
        auto previous_time = std::chrono::high_resolution_clock::now();
        float accumulated_time = 0.0f;
        int frame_count = 0;
        auto fps_timer = std::chrono::high_resolution_clock::now();

        // Main game loop
        while (game_manager_->IsRunning()) {
            // Calculate delta time
            auto current_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> delta_duration = current_time - previous_time;
            float delta_time = delta_duration.count();
            previous_time = current_time;

            // Clamp delta time to avoid large jumps
            delta_time = std::min(delta_time, MAX_FRAME_TIME);
            accumulated_time += delta_time;

            // Fixed timestep update
            while (accumulated_time >= FIXED_TIMESTEP) {
                // Update game with fixed timestep
                game_manager_->Update(FIXED_TIMESTEP);
                accumulated_time -= FIXED_TIMESTEP;
            }

            // Render with variable timestep (interpolation happens in renderer)
            game_manager_->Render();

            frame_count++;

            // Log FPS every second
            auto elapsed = std::chrono::high_resolution_clock::now() - fps_timer;
            if (elapsed >= std::chrono::seconds(1)) {
                std::cout << "FPS: " << frame_count << " | "
                          << "Frame Time: " << std::fixed << std::setprecision(2)
                          << (1000.0f / frame_count) << "ms" << std::endl;
                frame_count = 0;
                fps_timer = std::chrono::high_resolution_clock::now();
            }
        }
    }

    void Shutdown() {
        if (game_manager_) {
            std::cout << "Shutting down game..." << std::endl;
            game_manager_->Shutdown();
            std::cout << game_manager_->GetDebugInfo() << std::endl;
        }
    }

private:
    std::unique_ptr<GameManager> game_manager_;
};

/**
 * Main entry point for TopSpeed Racing Game
 */
int main(int argc, char* argv[]) {
    std::cout << "========================================" << std::endl;
    std::cout << "   TopSpeed Racing Game Engine v1.0     " << std::endl;
    std::cout << "   Phase 4 - Complete System Integration" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    GameApp app;

    // Initialize the game
    if (!app.Initialize()) {
        std::cerr << "Failed to initialize application!" << std::endl;
        return 1;
    }

    try {
        // Run the game loop
        app.Run();
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown exception caught!" << std::endl;
        return 1;
    }

    // Cleanup
    app.Shutdown();

    std::cout << "\nGame closed successfully!" << std::endl;
    return 0;
}
