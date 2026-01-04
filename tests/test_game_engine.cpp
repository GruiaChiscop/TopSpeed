#include <cassert>
#include <cmath>
#include <iostream>
#include <chrono>
#include <thread>
#include "../src/engine/GameEngine.h"
#include "../src/engine/GameClock.h"
#include "../src/engine/GameState.h"
#include "../src/audio/Effects.h"

using namespace TopSpeed;

// ============================================================================
// Test Utilities
// ============================================================================

class TestResults {
public:
    void Pass(const std::string& test_name) {
        passed_++;
        std::cout << "  ✓ " << test_name << std::endl;
    }
    
    void Fail(const std::string& test_name, const std::string& reason) {
        failed_++;
        std::cout << "  ✗ " << test_name << " - " << reason << std::endl;
    }
    
    void PrintSummary() {
        std::cout << "\n" << (passed_ + failed_) << " tests: "
                  << passed_ << " passed, " << failed_ << " failed\n";
    }
    
    int GetFailed() const { return failed_; }
    
private:
    int passed_ = 0;
    int failed_ = 0;
};

bool FloatEqual(float a, float b, float epsilon = 0.0001f) {
    return std::abs(a - b) < epsilon;
}

// ============================================================================
// GameClock Tests
// ============================================================================

void TestGameClock(TestResults& results) {
    std::cout << "\n=== Testing GameClock ===" << std::endl;
    
    GameClock clock;
    
    // Test 1: Initial state
    if (clock.GetFrameCount() == 0 && FloatEqual(clock.GetTotalTime(), 0.0f)) {
        results.Pass("Clock initial state");
    } else {
        results.Fail("Clock initial state", "Should start at frame 0, time 0");
    }
    
    // Test 2: Update increments frame
    clock.Update();
    if (clock.GetFrameCount() == 1) {
        results.Pass("Clock increments frame count");
    } else {
        results.Fail("Clock increments frame count",
                     "Frame count should increment");
    }
    
    // Test 3: Delta time is positive
    clock.Update();
    if (clock.GetDeltaTime() > 0.0f) {
        results.Pass("Clock delta time is positive");
    } else {
        results.Fail("Clock delta time is positive",
                     "Delta time should be > 0");
    }
    
    // Test 4: Total time increases
    float prev_total = clock.GetTotalTime();
    clock.Update();
    if (clock.GetTotalTime() >= prev_total) {
        results.Pass("Clock total time increases");
    } else {
        results.Fail("Clock total time increases",
                     "Total time should increase monotonically");
    }
    
    // Test 5: FPS calculation
    clock.Reset();
    // Simulate 60 frames
    for (int i = 0; i < 60; i++) {
        clock.Update();
    }
    float fps = clock.GetFrameRate();
    if (fps > 50.0f && fps < 70.0f) {  // Should be around 60
        results.Pass("FPS calculation reasonable");
    } else {
        results.Fail("FPS calculation reasonable",
                     "FPS should be around 60, got " + std::to_string(fps));
    }
    
    // Test 6: Pause/Resume
    clock.Reset();
    clock.Pause();
    float prev_time = clock.GetTotalTime();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    clock.Update();
    if (FloatEqual(clock.GetDeltaTime(), 0.0f, 0.001f)) {
        results.Pass("Clock pause stops delta time");
    } else {
        results.Fail("Clock pause stops delta time",
                     "Paused clock should have ~0 delta time");
    }
    
    // Test 7: Reset
    clock.Reset();
    if (clock.GetFrameCount() == 0 && FloatEqual(clock.GetTotalTime(), 0.0f)) {
        results.Pass("Clock reset works");
    } else {
        results.Fail("Clock reset works", "Reset should clear state");
    }
}

// ============================================================================
// GameState Tests
// ============================================================================

void TestGameState(TestResults& results) {
    std::cout << "\n=== Testing GameState ===" << std::endl;
    
    GameConfig config;
    
    // Test 1: Default configuration
    if (config.master_volume == 1.0f && config.player_name == "Player") {
        results.Pass("Config defaults");
    } else {
        results.Fail("Config defaults", "Should have default values");
    }
    
    // Test 2: Modify configuration
    config.master_volume = 0.8f;
    config.player_name = "Champion";
    config.difficulty = 2;
    if (config.master_volume == 0.8f && config.difficulty == 2) {
        results.Pass("Modify configuration");
    } else {
        results.Fail("Modify configuration", "Should store modified values");
    }
    
    // Test 3: Reset to defaults
    config.Reset();
    if (config.master_volume == 1.0f && config.player_name == "Player") {
        results.Pass("Reset configuration");
    } else {
        results.Fail("Reset configuration", "Should reset to defaults");
    }
    
    // Test 4: Save configuration
    config.master_volume = 0.9f;
    config.difficulty = 1;
    if (config.Save("test_config.tsp")) {
        results.Pass("Save configuration");
    } else {
        results.Fail("Save configuration", "Should save successfully");
    }
    
    // Test 5: Load configuration
    GameConfig loaded;
    loaded.Reset();  // Clear first
    if (loaded.Load("test_config.tsp")) {
        results.Pass("Load configuration");
    } else {
        results.Fail("Load configuration", "Should load successfully");
    }
    
    // Test 6: Verify loaded values
    if (FloatEqual(loaded.master_volume, 0.9f) && loaded.difficulty == 1) {
        results.Pass("Loaded configuration matches saved");
    } else {
        results.Fail("Loaded configuration matches saved",
                     "Loaded values should match saved values");
    }
}

// ============================================================================
// GameEngine Tests
// ============================================================================

void TestGameEngineBasics(TestResults& results) {
    std::cout << "\n=== Testing GameEngine Basics ===" << std::endl;
    
    GameEngine engine;
    
    // Test 1: Not initialized by default
    if (!engine.IsInitialized()) {
        results.Pass("Engine not initialized by default");
    } else {
        results.Fail("Engine not initialized by default",
                     "Should not be initialized");
    }
    
    // Test 2: Initialize
    if (engine.Initialize()) {
        results.Pass("Engine initialization succeeds");
    } else {
        results.Fail("Engine initialization succeeds",
                     "Should initialize successfully");
    }
    
    // Test 3: After initialization
    if (engine.IsInitialized() && engine.IsRunning()) {
        results.Pass("Engine running after initialization");
    } else {
        results.Fail("Engine running after initialization",
                     "Should be initialized and running");
    }
    
    // Test 4: Game state
    if (engine.GetGameState() == GameState::MainMenu) {
        results.Pass("Engine starts in MainMenu");
    } else {
        results.Fail("Engine starts in MainMenu",
                     "Should start in MainMenu state");
    }
    
    // Test 5: Shutdown
    engine.Shutdown();
    if (!engine.IsInitialized()) {
        results.Pass("Engine shutdown works");
    } else {
        results.Fail("Engine shutdown works", "Should be shutdown");
    }
}

void TestGameEngineControl(TestResults& results) {
    std::cout << "\n=== Testing GameEngine Control ===" << std::endl;
    
    GameEngine engine;
    engine.Initialize();
    
    // Test 1: Change game state
    engine.SetGameState(GameState::RaceActive);
    if (engine.GetGameState() == GameState::RaceActive) {
        results.Pass("Change game state");
    } else {
        results.Fail("Change game state", "Should change state");
    }
    
    // Test 2: Pause/Resume
    if (!engine.IsPaused()) {
        engine.TogglePause();
        if (engine.IsPaused()) {
            results.Pass("Toggle pause on");
        } else {
            results.Fail("Toggle pause on", "Should pause");
        }
        
        engine.TogglePause();
        if (!engine.IsPaused()) {
            results.Pass("Toggle pause off");
        } else {
            results.Fail("Toggle pause off", "Should unpause");
        }
    }
    
    // Test 3: Game speed control
    engine.SetGameSpeed(0.5f);
    if (engine.GetGameSpeed() == 0.5f) {
        results.Pass("Set game speed");
    } else {
        results.Fail("Set game speed", "Should set speed");
    }
    
    // Test 4: Game speed clamping (no negative)
    engine.SetGameSpeed(-1.0f);
    if (engine.GetGameSpeed() >= 0.0f) {
        results.Pass("Game speed clamping min");
    } else {
        results.Fail("Game speed clamping min",
                     "Should not allow negative speed");
    }
    
    // Test 5: Debug mode
    engine.SetDebugMode(true);
    if (engine.IsDebugMode()) {
        results.Pass("Set debug mode");
    } else {
        results.Fail("Set debug mode", "Should set debug mode");
    }
    
    engine.Shutdown();
}

void TestGameEngineConfiguration(TestResults& results) {
    std::cout << "\n=== Testing GameEngine Configuration ===" << std::endl;
    
    GameEngine engine;
    engine.Initialize();
    
    // Test 1: Get config
    GameConfig config = engine.GetConfig();
    if (config.master_volume == 1.0f) {
        results.Pass("Get engine configuration");
    } else {
        results.Fail("Get engine configuration", "Should return config");
    }
    
    // Test 2: Update config
    config.master_volume = 0.7f;
    config.player_name = "Player1";
    engine.UpdateConfig(config);
    
    GameConfig updated = engine.GetConfig();
    if (updated.master_volume == 0.7f) {
        results.Pass("Update engine configuration");
    } else {
        results.Fail("Update engine configuration",
                     "Config should be updated");
    }
    
    engine.Shutdown();
}

// ============================================================================
// Integration Tests
// ============================================================================

void TestEngineIntegration(TestResults& results) {
    std::cout << "\n=== Testing Engine Integration ===" << std::endl;
    
    GameEngine engine;
    
    // Test 1: Complete initialization cycle
    if (!engine.Initialize()) {
        results.Fail("Engine initialization", "Should initialize");
        return;
    }
    results.Pass("Engine initialization cycle");
    
    // Test 2: Get subsystems
    auto& audio = engine.GetAudioEngine();
    auto& mixer_mgr = engine.GetMixerManager();
    auto& input = engine.GetInputManager();
    
    results.Pass("Access audio engine");
    results.Pass("Access mixer manager");
    results.Pass("Access input manager");
    
    // Test 3: Create mixers
    SoundMixer* music = mixer_mgr.CreateMixer("music");
    if (music != nullptr) {
        results.Pass("Create audio mixer");
    } else {
        results.Fail("Create audio mixer", "Should create mixer");
    }
    
    // Test 4: Add audio effects
    if (music != nullptr) {
        music->AddEffect(std::make_unique<ReverbEffect>(50.0f, 0.3f));
        if (music->GetEffectCount() == 1) {
            results.Pass("Add audio effects");
        } else {
            results.Fail("Add audio effects", "Should add effect");
        }
    }
    
    // Test 5: Setup input
    input.MapKey("quit", SDL_SCANCODE_ESCAPE);
    if (input.GetMappedKey("quit") == SDL_SCANCODE_ESCAPE) {
        results.Pass("Setup input bindings");
    } else {
        results.Fail("Setup input bindings", "Should map key");
    }
    
    // Test 6: Game loop cycle
    bool loop_success = true;
    for (int i = 0; i < 5; i++) {
        if (!engine.Update()) {
            loop_success = false;
            break;
        }
    }
    if (loop_success) {
        results.Pass("Game loop update cycle");
    } else {
        results.Fail("Game loop update cycle",
                     "Updates should succeed");
    }
    
    // Test 7: Shutdown cycle
    engine.Shutdown();
    if (!engine.IsInitialized()) {
        results.Pass("Complete shutdown cycle");
    } else {
        results.Fail("Complete shutdown cycle",
                     "Should shutdown completely");
    }
}

void TestEngineDebugInfo(TestResults& results) {
    std::cout << "\n=== Testing Engine Debug Output ===" << std::endl;
    
    GameEngine engine;
    engine.Initialize();
    
    // Test 1: Get debug info
    std::string debug_info = engine.GetDebugInfo();
    if (!debug_info.empty()) {
        results.Pass("Get engine debug info");
    } else {
        results.Fail("Get engine debug info", "Should return debug info");
    }
    
    // Test 2: Debug info contains expected content
    if (debug_info.find("TopSpeed Engine") != std::string::npos) {
        results.Pass("Debug info has expected format");
    } else {
        results.Fail("Debug info has expected format",
                     "Should contain engine name");
    }
    
    engine.Shutdown();
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
    std::cout << "\n" << "="*60 << std::endl;
    std::cout << "TopSpeed Game Engine Test Suite" << std::endl;
    std::cout << "="*60 << std::endl;
    
    TestResults results;
    
    // Run all tests
    TestGameClock(results);
    TestGameState(results);
    TestGameEngineBasics(results);
    TestGameEngineControl(results);
    TestGameEngineConfiguration(results);
    TestEngineIntegration(results);
    TestEngineDebugInfo(results);
    
    // Print summary
    results.PrintSummary();
    
    std::cout << "\n" << "="*60 << std::endl;
    if (results.GetFailed() == 0) {
        std::cout << "✓ ALL TESTS PASSED" << std::endl;
    } else {
        std::cout << "✗ SOME TESTS FAILED" << std::endl;
    }
    std::cout << "="*60 << std::endl;
    
    return results.GetFailed();
}
