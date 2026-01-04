# TopSpeed Phase 3 - Quick Reference Guide

**Branch:** `modernize/sdl3-miniaudio`  
**Status:** Phase 3 Week 1 Complete ✅

---

## 📋 Files Overview

### Headers (Include These)

```cpp
#include "audio/Effects.h"           // Reverb, EQ, Compressor, Distortion, Flange
#include "audio/SoundMixer.h"        // Sound mixing and effect chains
#include "input/InputManager.h"      // SDL3 input abstraction
#include "engine/GameClock.h"        // High-resolution timing
#include "engine/GameState.h"        // Game state machine & config
#include "engine/GameEngine.h"       // Main engine coordination
```

### Namespaces

All code is in `namespace TopSpeed { }`

---

## 🎵 Audio System

### Creating Mixers

```cpp
// Create named mixer
SoundMixer* music = mixer_manager.CreateMixer("music");
SoundMixer* sfx = mixer_manager.CreateMixer("sfx");

// Standard mixer names (constants available)
SoundMixer* m = mixer_manager.CreateMixer(MixerManager::MIXER_MUSIC);
// Options: MIXER_MUSIC, MIXER_SFX, MIXER_UI, MIXER_DIALOGUE
```

### Adding Effects

```cpp
// Reverb (delay_ms, feedback)
music->AddEffect(std::make_unique<ReverbEffect>(50.0f, 0.5f));

// Equalizer (3-band)
auto eq = std::make_unique<EqualizerEffect>();
eq->SetBandGain(0, 6.0f);    // Low: boost 6dB
eq->SetBandGain(1, 0.0f);    // Mid: neutral
eq->SetBandGain(2, -3.0f);   // High: reduce 3dB
music->AddEffect(std::move(eq));

// Compressor (threshold, ratio, attack, release)
music->AddEffect(std::make_unique<CompressorEffect>(0.5f, 4.0f, 0.1f, 0.2f));

// Distortion (drive amount 0.0-1.0)
sfx->AddEffect(std::make_unique<DistortionEffect>(0.7f));

// Flange (rate, depth)
music->AddEffect(std::make_unique<FlangeEffect>(2.0f, 0.5f));
```

### Managing Sounds

```cpp
// Add sound to mixer
music->AddSound(bgm_sound_id);
music->AddSound(ambient_sound_id);

// Check if mixer has sound
if (music->HasSound(bgm_sound_id)) {
    // Sound is in mixer
}

// Remove sound
music->RemoveSound(ambient_sound_id);

// Clear all sounds
music->ClearSounds();
```

### Volume Control

```cpp
// Set immediate volume (0.0 - 1.0)
music->SetVolume(0.8f);

// Fade to target volume over duration_ms
music->FadeVolume(0.5f, 2000);  // Fade to 50% in 2 seconds

// Control status
music->SetEnabled(true);        // Enable/disable mixer
music->SetMuted(false);         // Mute/unmute
music->SetSolo(true);           // Debug: solo this mixer
```

### Master Volume

```cpp
// Set master volume for all mixers
mixer_manager.SetMasterVolume(0.9f);

// Get value
float master = mixer_manager.GetMasterVolume();
```

---

## ⌨️ Input System (SDL3)

### Key Binding

```cpp
// Map action name to SDL scancode
input.MapKey("accelerate", SDL_SCANCODE_UP);
input.MapKey("brake", SDL_SCANCODE_DOWN);
input.MapKey("pause", SDL_SCANCODE_ESCAPE);
input.MapKey("nitro", SDL_SCANCODE_SPACE);

// Get mapped key
SDL_Scancode key = input.GetMappedKey("accelerate");

// Clear all bindings
input.ClearKeyBindings();
```

### Checking Key States

```cpp
// Frame-wide state (held down)
if (input.IsKeyDown("accelerate")) {
    car.Accelerate();
}

// Just pressed this frame
if (input.IsKeyPressed("nitro")) {
    car.UseNitro();
}

// Just released this frame
if (input.IsKeyReleased("brake")) {
    car.OffBrake();
}

// Raw scancode check (without binding)
if (input.IsScancodeDown(SDL_SCANCODE_P)) {
    // P key is held
}
```

### Axis Binding (Joystick/Gamepad)

```cpp
// Map action to joystick axis
// Parameters: action, joystick_index, axis, invert
input.MapAxis("steering", 0, 0, false);  // Joystick 0, X axis
input.MapAxis("throttle", 0, 5, false); // Joystick 0, Right trigger

// Get axis value (-1.0 to 1.0)
float steering = input.GetAxisValue("steering");

// Get axis with dead zone applied
float steering = input.GetAxisValueDeadZone("steering", 0.15f);

// Set global dead zone
input.SetDeadZone(0.15f);
```

### Mouse

```cpp
// Position
int x = input.GetMouseX();
int y = input.GetMouseY();

// Movement this frame
int dx = input.GetMouseDeltaX();
int dy = input.GetMouseDeltaY();

// Button states (SDL button constants)
if (input.IsMouseButtonDown(SDL_BUTTON_LEFT)) {
    // Left click held
}

if (input.IsMouseButtonPressed(SDL_BUTTON_RIGHT)) {
    // Right click just pressed
}

// Control mouse
input.SetMouseVisible(true);   // Show/hide cursor
input.SetMouseCapture(true);   // Lock to window
```

### Text Input

```cpp
// Enable text input for UI
input.StartTextInput();

// Get text entered this frame
std::string text = input.GetTextInput();

// Disable text input
input.StopTextInput();
```

### Gamepad

```cpp
// Check connection
if (input.IsGamepadConnected(0)) {
    // Joystick 0 is connected
}

// Rumble feedback
input.VibrationFeedback(0, 0.5f, 0.8f, 200);  // Controller 0, left 50%, right 80%, 200ms
```

### Callbacks

```cpp
// Set key change callback
input.SetKeyCallback([](const std::string& action, bool pressed) {
    std::cout << action << ": " << (pressed ? "down" : "up");
});

// Set axis change callback
input.SetAxisCallback([](const std::string& action, float value) {
    std::cout << action << ": " << value;
});
```

---

## ⏱️ Game Clock

```cpp
// Update clock (call once per frame)
game_clock.Update();

// Get timing info
float delta = game_clock.GetDeltaTime();    // Time since last frame (seconds)
float total = game_clock.GetTotalTime();    // Total elapsed time (seconds)
uint64_t frame = game_clock.GetFrameCount(); // Frame number
float fps = game_clock.GetFrameRate();      // Rolling FPS average

// Frame statistics
float min_time = game_clock.GetMinFrameTime();  // Fastest frame (seconds)
float max_time = game_clock.GetMaxFrameTime();  // Slowest frame (seconds)
float avg_time = game_clock.GetAvgFrameTime();  // Average (60-frame window)

// Control
game_clock.Pause();
game_clock.Resume();
bool paused = game_clock.IsPaused();

// Reset
game_clock.Reset();

// Debug output
std::cout << game_clock.GetDebugInfo();
```

---

## 🎮 Game Engine

### Initialization

```cpp
GameEngine engine;

// Initialize all subsystems
if (!engine.Initialize()) {
    // Error handling
}

// Check status
if (!engine.IsInitialized()) {
    // Not ready
}
```

### Main Loop

```cpp
while (engine.IsRunning()) {
    // Perform game update
    engine.Update();
    
    // Render would go here (placeholder for rendering layer)
    // RenderGame();
}

engine.Shutdown();
```

### Game State

```cpp
// Change state (with automatic announcement)
engine.SetGameState(GameState::RaceActive);
engine.SetGameState(GameState::RacePaused);
engine.SetGameState(GameState::MainMenu);

// Get current state
GameState state = engine.GetGameState();

// Check state
if (engine.GetGameState() == GameState::RaceActive) {
    // Game is running
}
```

### Control

```cpp
// Pause/Resume (with announcement)
engine.TogglePause();

// Check pause state
if (engine.IsPaused()) {
    // Game is paused
}

// Speed control (for slow-motion, fast-forward)
engine.SetGameSpeed(0.5f);  // Slow motion
engine.SetGameSpeed(1.0f);  // Normal
engine.SetGameSpeed(2.0f);  // Fast forward
```

### Configuration

```cpp
// Get current config
GameConfig config = engine.GetConfig();

// Modify
config.master_volume = 0.9f;
config.player_name = "Champion";
config.difficulty = 2;

// Apply changes
engine.UpdateConfig(config);

// Save to file
config.Save("profile.tsp");

// Load from file
GameConfig loaded;
loaded.Load("profile.tsp");
engine.UpdateConfig(loaded);
```

### Debug

```cpp
// Enable debug mode
engine.SetDebugMode(true);

// Get comprehensive debug info
std::cout << engine.GetDebugInfo();

// Debug audio
mixer_manager.SetDebugMode(true);

// Print game clock stats
std::cout << game_clock.GetDebugInfo();
```

### Event Callbacks

```cpp
// Set engine event callback
engine.SetEventCallback([](EngineEvent event) {
    switch (event) {
        case EngineEvent::Initialized:
            std::cout << "Engine initialized\n";
            break;
        case EngineEvent::GameStateChanged:
            std::cout << "Game state changed\n";
            break;
        case EngineEvent::PauseToggled:
            std::cout << "Pause toggled\n";
            break;
        case EngineEvent::SpeedChanged:
            std::cout << "Speed changed\n";
            break;
        default:
            break;
    }
});
```

---

## 🎛️ Game Configuration

### Audio Settings

```cpp
GameConfig config;

// Volume levels (0.0 - 1.0)
config.master_volume = 1.0f;
config.music_volume = 0.8f;
config.sfx_volume = 0.9f;
config.ui_volume = 0.7f;
config.dialogue_volume = 1.0f;
config.audio_enabled = true;
```

### Accessibility

```cpp
config.screen_reader_enabled = true;  // Enable/disable screen reader
config.tts_enabled = true;            // Text-to-speech
config.speech_rate = 1.0f;            // Speech speed multiplier
config.speech_volume = 1.0f;          // Speech volume (0.0 - 1.0)
```

### Input

```cpp
config.controller_dead_zone = 0.15f;  // Joystick dead zone
config.controller_rumble = true;      // Rumble feedback
config.invert_steering = false;       // Invert Y axis
config.steering_sensitivity = 1.0f;   // Steering response
```

### Game Settings

```cpp
config.difficulty = 1;                // 0=easy, 1=normal, 2=hard
config.multiplayer_enabled = true;    // Enable multiplayer
config.player_name = "Player";        // Player name
config.network_port = 9999;           // Network port
```

### Display

```cpp
config.display_width = 1280;          // Screen width
config.display_height = 720;          // Screen height
config.fullscreen = false;            // Fullscreen mode
config.target_fps = 60;               // Target framerate
```

---

## 🔧 Typical Setup

```cpp
#include "engine/GameEngine.h"
#include "audio/Effects.h"

using namespace TopSpeed;

int main() {
    // Create engine
    GameEngine engine;
    
    // Initialize all subsystems
    if (!engine.Initialize()) {
        std::cerr << "Failed to initialize engine" << std::endl;
        return 1;
    }
    
    // Get managers
    auto& audio = engine.GetAudioEngine();
    auto& mixer_mgr = engine.GetMixerManager();
    auto& input = engine.GetInputManager();
    
    // Setup audio
    SoundMixer* music = mixer_mgr.CreateMixer("music");
    music->AddEffect(std::make_unique<ReverbEffect>(50.0f, 0.3f));
    
    // Setup input
    input.MapKey("quit", SDL_SCANCODE_ESCAPE);
    input.MapKey("pause", SDL_SCANCODE_P);
    input.SetDeadZone(0.15f);
    
    // Main loop
    while (engine.IsRunning()) {
        // Handle quit
        if (input.IsKeyPressed("quit")) {
            engine.SetRunning(false);
        }
        
        // Update engine (handles all subsystems)
        engine.Update();
        
        // Game rendering goes here
        // RenderGame();
    }
    
    // Cleanup
    engine.Shutdown();
    
    return 0;
}
```

---

## 📊 Enum Reference

### GameState

```cpp
enum class GameState {
    MainMenu,
    Settings,
    SettingsAudio,
    SettingsControls,
    SettingsAccessibility,
    RaceLoading,
    RaceCountdown,
    RaceActive,
    RacePaused,
    RaceFinished,
    MultiplayerLobby,
    MultiplayerSync,
    Loading,
    Error,
    ShuttingDown
};
```

### EngineEvent

```cpp
enum class EngineEvent {
    Initialized,
    ShutdownStarted,
    ShutdownComplete,
    GameStateChanged,
    PauseToggled,
    SpeedChanged
};
```

---

## 🐛 Debugging Tips

```cpp
// Enable all debug output
game_clock.PrintDebugInfo();
engine.PrintDebugInfo();
mixer_mgr.PrintDebugInfo();
input.PrintDebugInfo();

// Check timing
float fps = game_clock.GetFrameRate();
if (fps < 55.0f) {
    std::cout << "Warning: FPS dropping (" << fps << ")\n";
}

// Monitor frame time
float frame_time_ms = game_clock.GetAvgFrameTime() * 1000.0f;
if (frame_time_ms > 16.6f) {
    std::cout << "Frame time: " << frame_time_ms << "ms (budget: 16.6ms)\n";
}

// Check audio load
int mixer_count = mixer_mgr.GetMixerCount();
std::cout << "Active mixers: " << mixer_count << "\n";
```

---

## 📚 Additional Resources

- See `PHASE3_WEEK1_COMPLETE.md` for architecture details
- See header files for comprehensive documentation
- See `src/` for full implementation

---

**Last Updated:** 2026-01-04  
**Status:** Phase 3 Week 1 Complete ✅
