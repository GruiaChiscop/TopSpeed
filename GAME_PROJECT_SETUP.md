# TopSpeed Game - Project Recreation

**Project Status:** Phase 4 - Game Development  
**Branch:** `game/topspeed-recreation`  
**Base Engine:** Modernized SDL3 + miniaudio  
**Start Date:** 2026-01-04  

---

## 🎆 Project Overview

This document outlines the recreation of TopSpeed using the modern game engine we've built. The goal is to leverage all the engine features (audio, input, game loop, state management) to recreate the classic TopSpeed game with enhanced capabilities.

---

## 💳 Architecture

### Directory Structure

```
TopSpeed/
├─ src/
│  ├─ engine/              ✓ (Complete - from modernize branch)
│  ├─ audio/               ✓ (Complete - from modernize branch)
│  ├─ input/               ✓ (Complete - from modernize branch)
│  ├─ network/             ✓ (Complete - from modernize branch)
│  ├─ accessibility/       ✓ (Complete - from modernize branch)
│  ├─ game/                🔰 NEW - Game-specific code
│  │  ├─ GameWorld.h       (World/track management)
│  │  ├─ GameWorld.cpp
│  │  ├─ Player.h            (Player vehicle)
│  │  ├─ Player.cpp
│  │  ├─ Vehicle.h           (Vehicle physics)
│  │  ├─ Vehicle.cpp
│  │  ├─ Track.h             (Track data & management)
│  │  ├─ Track.cpp
│  │  ├─ Obstacle.h          (Track obstacles)
│  │  ├─ Obstacle.cpp
│  │  ├─ GameLogic.h         (Main game logic)
│  │  ├─ GameLogic.cpp
│  │  ├─ UI.h                (UI system)
│  │  ├─ UI.cpp
│  │  ├─ Renderer.h          (Graphics abstraction)
│  │  ├─ Renderer.cpp
│  │  ├─ AudioManager.h      (Game-level audio)
│  │  ├─ AudioManager.cpp
│  │  ├─ GameState.h         (Game-specific states)
│  │  ├─ GameState.cpp
│  └─ main.cpp            🔰 NEW - Game entry point
├─ assets/               🔰 NEW
│  ├─ data/
│  │  ├─ tracks/           (Track definitions)
│  │  ├─ vehicles/         (Vehicle configs)
│  │  ├─ levels/           (Level data)
│  │  ├─ ui/                (UI layouts)
│  │  ├─ config/           (Game config files)
│  ├─ audio/                🔰 NEW
│  │  ├─ music/
│  │  ├─ sfx/
│  │  ├─ ambience/
│  ├─ graphics/             🔰 NEW (optional - depends on renderer)
│  └─ fonts/                🔰 NEW (for UI)
├─ tests/                🔰 NEW - Game tests
│  ├─ test_game_logic.cpp
│  ├─ test_vehicles.cpp
│  ├─ test_tracks.cpp
│  ├─ test_ui.cpp
├─ docs/                 (Game documentation)
├─ README.md
├─ GAME_ARCHITECTURE.md  (Game design document)
├─ DEVELOPMENT_LOG.md    (Progress tracking)
└─ CMakeLists.txt        (Build configuration)
```

---

## 🌟 Core Game Components

### 1. Player & Vehicle System

```cpp
// src/game/Vehicle.h
class Vehicle {
public:
    // Position & movement
    Vector2D position;
    float rotation = 0.0f;      // in degrees
    Vector2D velocity;
    float angular_velocity = 0.0f;
    
    // Vehicle properties
    float max_speed = 200.0f;   // units/second
    float acceleration = 150.0f;
    float deceleration = 200.0f;
    float turn_speed = 180.0f;  // degrees/second
    float mass = 1.0f;
    
    // State
    enum State {
        Idle,
        Accelerating,
        Braking,
        Sliding,
        Crashed
    } state = Idle;
    
    // Methods
    void Update(float deltaTime);
    void Accelerate(float factor);  // 0.0 to 1.0
    void Brake(float factor);
    void Turn(float direction);     // -1.0 to 1.0
    void ApplyFriction(float friction);
    bool CheckCollision(const Obstacle& obstacle);
};

class Player {
public:
    Vehicle vehicle;
    int lives = 3;
    int score = 0;
    float current_distance = 0.0f;
    
    void Update(float deltaTime, const InputManager& input);
    void TakeDamage();
    void Reset();
};
```

### 2. Track & World System

```cpp
// src/game/Track.h
class Obstacle {
public:
    Vector2D position;
    Vector2D size;
    enum Type { Car, Truck, Wall, Debris } type;
    float speed = 0.0f;  // for moving obstacles
    
    bool CheckCollision(const Vehicle& vehicle) const;
};

class Track {
public:
    std::string name;
    std::vector<Obstacle> obstacles;
    float track_length = 1000.0f;
    Vector2D track_center = {640.0f, 360.0f};
    Vector2D track_width = {400.0f, 480.0f};
    
    // Difficulty
    enum Difficulty { Easy, Normal, Hard, Expert } difficulty;
    float obstacle_frequency = 0.5f;  // obstacles per second
    float speed_modifier = 1.0f;      // affects obstacle speeds
    
    void Update(float deltaTime);
    void GenerateObstacles(float elapsed_time);
    std::vector<Obstacle>& GetVisibleObstacles(const Vector2D& view);
};

class GameWorld {
public:
    Track current_track;
    Player player;
    
    void Update(float deltaTime, const InputManager& input);
    void Reset();
};
```

### 3. Game Logic & State

```cpp
// src/game/GameLogic.h
class GameLogic {
public:
    enum GameMode {
        Menu,
        Gameplay,
        Paused,
        GameOver,
        LevelComplete,
        Settings,
        HighScores
    } current_mode = Menu;
    
    void Update(float deltaTime, GameEngine& engine);
    void Render(Renderer& renderer);
    
    void StartGame(int difficulty);
    void PauseGame();
    void ResumeGame();
    void EndGame();
    void RestartLevel();
    
private:
    GameWorld world;
    float level_timer = 0.0f;
};
```

### 4. Audio System (Game-Level)

```cpp
// src/game/AudioManager.h
class GameAudioManager {
public:
    void Initialize(AudioEngine& engine);
    
    // Background music
    void PlayMenuMusic();
    void PlayGameplayMusic();
    void PlayPauseMusic();
    void StopMusic();
    
    // SFX
    void PlayAccelerationSound(float pitch);
    void PlayBrakeSound();
    void PlayCollisionSound(float intensity);
    void PlayObstaclePassSound();
    void PlayPowerupSound();
    void PlayGameOverSound();
    
    // Ambience
    void PlayAmbience(const std::string& ambience_type);
    
    void Update(float deltaTime);
    
private:
    MixerManager* mixer_manager;
};
```

### 5. UI System

```cpp
// src/game/UI.h
class UIElement {
public:
    virtual ~UIElement() = default;
    virtual void Update(float deltaTime) = 0;
    virtual void Render(Renderer& renderer) = 0;
    virtual bool HandleInput(const InputManager& input) = 0;
};

class HUD : public UIElement {
public:
    // Display game info
    void SetScore(int score);
    void SetLives(int lives);
    void SetDistance(float distance);
    void SetSpeed(float speed);
    void SetTime(float time);
    
    void Update(float deltaTime) override;
    void Render(Renderer& renderer) override;
    bool HandleInput(const InputManager& input) override;
};

class MenuUI : public UIElement {
public:
    enum Selection { Play, Settings, HighScores, Quit };
    Selection current_selection = Play;
    
    void Update(float deltaTime) override;
    void Render(Renderer& renderer) override;
    bool HandleInput(const InputManager& input) override;
};
```

---

## 📚 Implementation Roadmap

### Phase 4.1: Core Game Objects (2 hours)

```
[ ] Vehicle class
    [ ] Position/rotation system
    [ ] Movement and acceleration
    [ ] Collision detection
    [ ] Visual representation
    
[ ] Player wrapper
    [ ] Score system
    [ ] Lives management
    [ ] Input mapping
    
[ ] Track system
    [ ] Obstacle management
    [ ] Dynamic obstacle generation
    [ ] Track difficulty levels
```

### Phase 4.2: Game Logic (2 hours)

```
[ ] GameLogic class
    [ ] Game state machine
    [ ] Win/lose conditions
    [ ] Score calculation
    [ ] Level progression
    
[ ] Physics system
    [ ] Collision responses
    [ ] Friction and drag
    [ ] Speed limits
    
[ ] Difficulty system
    [ ] Obstacle spawning
    [ ] Speed scaling
    [ ] Time-based progression
```

### Phase 4.3: Audio Integration (1 hour)

```
[ ] Music system
    [ ] Menu music
    [ ] Gameplay music
    [ ] Pause/transition music
    
[ ] Sound effects
    [ ] Engine sounds
    [ ] Collision sounds
    [ ] UI sounds
    
[ ] Mixer management
    [ ] Music mixer
    [ ] SFX mixer
    [ ] Master volume
```

### Phase 4.4: Input & Controls (1 hour)

```
[ ] Input mapping
    [ ] Accelerate/brake
    [ ] Turn left/right
    [ ] Pause menu
    [ ] Menu navigation
    
[ ] Gamepad support
    [ ] Analog stick movement
    [ ] Button mappings
    [ ] Rumble feedback
```

### Phase 4.5: UI System (2 hours)

```
[ ] HUD (Heads-Up Display)
    [ ] Score display
    [ ] Lives counter
    [ ] Speed indicator
    [ ] Distance traveled
    [ ] Time remaining
    
[ ] Menu system
    [ ] Main menu
    [ ] Pause menu
    [ ] Game over screen
    [ ] Settings menu
    [ ] High scores screen
```

### Phase 4.6: Rendering Abstraction (1 hour)

```
[ ] Renderer interface
    [ ] Drawing primitives
    [ ] Text rendering
    [ ] Sprite handling
    [ ] Screen management
    
[ ] Game world rendering
    [ ] Track rendering
    [ ] Vehicle rendering
    [ ] Obstacle rendering
    [ ] HUD rendering
```

### Phase 4.7: Integration & Testing (1.5 hours)

```
[ ] Full game integration
    [ ] Engine + game code
    [ ] Audio system integration
    [ ] Input system integration
    [ ] Rendering pipeline
    
[ ] Testing
    [ ] Unit tests for game systems
    [ ] Integration tests
    [ ] Play testing
    [ ] Bug fixes
```

### Phase 4.8: Polish & Release (1 hour)

```
[ ] Performance optimization
[ ] Visual polish
[ ] Audio mixing
[ ] Bug fixes
[ ] Documentation
```

---

## 💻 Key Implementation Decisions

### Rendering Backend

**Decision:** SDL3 with 2D rendering (simple canvas-style approach)

**Rationale:**
- No external rendering library dependency
- SDL3 has built-in drawing APIs
- Sufficient for classic TopSpeed-style gameplay
- Easy to extend to OpenGL/Vulkan later

**Alternative:** Use a proper graphics library (SDL_gpu, Raylib)

### Physics Model

**Decision:** Simple arcade physics (not realistic)

**Rationale:**
- TopSpeed is an arcade game, not a simulator
- Easier to balance gameplay
- Faster computations
- More fun for players

**Implementation:**
```cpp
// Arcade physics approach
velocity += acceleration * deltaTime;
velocity = Clamp(velocity, 0, max_speed);
position += velocity * deltaTime;
```

### Collision Detection

**Decision:** Axis-Aligned Bounding Box (AABB) collision

**Rationale:**
- Fast and simple
- Works well for game objects
- No complex geometry needed

### Audio Integration

**Decision:** Use the engine's mixer system with sound groups

**Rationale:**
- Centralized audio management
- Built-in volume control
- Effect chain support
- Perfect for our engine design

---

## 📋 Code Examples

### Main Game Loop

```cpp
// src/main.cpp
int main() {
    GameEngine engine;
    if (!engine.Initialize()) {
        return 1;
    }
    
    GameLogic game_logic;
    game_logic.Initialize(engine);
    
    auto& clock = engine.GetGameClock();
    auto& input = engine.GetInputManager();
    
    while (engine.IsRunning()) {
        engine.Update();
        
        float deltaTime = clock.GetDeltaTime();
        
        // Update game logic
        game_logic.Update(deltaTime, engine);
        
        // Render
        game_logic.Render();
        
        // Optional: Display debug info
        if (engine.IsDebugMode()) {
            std::cout << "FPS: " << clock.GetFrameRate() << std::endl;
        }
    }
    
    engine.Shutdown();
    return 0;
}
```

### Vehicle Update

```cpp
void Vehicle::Update(float deltaTime) {
    // Apply acceleration/braking
    if (accelerating) {
        velocity.y -= acceleration * deltaTime;
    } else if (braking) {
        velocity.y += deceleration * deltaTime;
    } else {
        // Natural deceleration
        velocity.y += friction * deltaTime;
    }
    
    // Clamp to max speed
    float current_speed = std::abs(velocity.y);
    if (current_speed > max_speed) {
        velocity.y = (velocity.y > 0 ? 1 : -1) * max_speed;
    }
    
    // Apply rotation based on turn input
    rotation += turn_input * turn_speed * deltaTime;
    
    // Update position
    position += velocity * deltaTime;
    
    // Keep in bounds
    position.x = Clamp(position.x, 
        track_center.x - track_width.x/2, 
        track_center.x + track_width.x/2);
}
```

### Obstacle Collision

```cpp
bool Vehicle::CheckCollision(const Obstacle& obstacle) const {
    // AABB collision
    return !(position.x + size.x/2 < obstacle.position.x - obstacle.size.x/2 ||
             position.x - size.x/2 > obstacle.position.x + obstacle.size.x/2 ||
             position.y + size.y/2 < obstacle.position.y - obstacle.size.y/2 ||
             position.y - size.y/2 > obstacle.position.y + obstacle.size.y/2);
}
```

---

## 🏗️ Build Configuration

### CMakeLists.txt (Game Target)

```cmake
# Game executable
add_executable(topspeed
    src/main.cpp
    src/game/GameLogic.cpp
    src/game/GameWorld.cpp
    src/game/Player.cpp
    src/game/Vehicle.cpp
    src/game/Track.cpp
    src/game/Obstacle.cpp
    src/game/UI.cpp
    src/game/AudioManager.cpp
    src/game/Renderer.cpp
    src/game/GameState.cpp
    # Engine sources from previous phases
    src/engine/GameEngine.cpp
    src/engine/GameClock.cpp
    src/engine/GameState.cpp
    src/audio/AudioEngine.cpp
    src/audio/SoundDevice.cpp
    src/audio/Effects.cpp
    src/audio/SoundMixer.cpp
    src/input/InputManager.cpp
)

target_link_libraries(topspeed
    SDL3::SDL3
    miniaudio
    pthread
)
```

---

## 🌟 Development Timeline

**Expected Duration:** ~10-12 hours  
**Milestones:**

```
Week 1: 
  [ ] Game architecture finalized
  [ ] Core game objects implemented
  [ ] Basic game loop working
  
Week 2:
  [ ] Full game logic implemented
  [ ] Audio integration complete
  [ ] Input system fully utilized
  
Week 3:
  [ ] UI system complete
  [ ] Rendering pipeline done
  [ ] Game fully playable
  [ ] Testing and bug fixes
```

---

## ✅ Success Criteria

- [☑] Game runs without crashes
- [☑] Player can control vehicle
- [☑] Obstacles spawn and move correctly
- [☑] Collision detection works
- [☑] Score system functional
- [☑] Audio plays correctly (engine sound, music)
- [☑] UI displays all relevant info
- [☑] Game states (menu, play, pause, game over) work
- [☑] Performance is smooth (60 FPS)
- [☑] No memory leaks

---

## 🚀 Next Steps

1. **Review this architecture** - Any feedback?
2. **Start Phase 4.1** - Implement Vehicle and Player classes
3. **Integrate with engine** - Connect to GameEngine
4. **Progressive development** - Each phase adds more features
5. **Regular testing** - Ensure each system works with the engine

---

**Status:** 🟢 Ready to Begin  
**Next Update:** After Phase 4.1 completion
