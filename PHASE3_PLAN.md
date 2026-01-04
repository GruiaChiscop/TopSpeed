# TopSpeed Modernization: Phase 3 - Integration & Testing

**Inspired by:** [NVGT (Nonvisual Gaming Toolkit)](https://github.com/samtupy/nvgt) architecture

**Date:** 2026-01-04  
**Status:** Planning Phase  
**Target Duration:** 2-3 weeks  

---

## Overview

Phase 3 focuses on integrating the Phase 2 subsystems (Audio, Networking, Accessibility) into a cohesive game engine following NVGT's proven architectural patterns:

- **Sound Mixer System** - Similar to NVGT's sound chains/effects pipeline
- **Input Management** - SDL3 keyboard/mouse handlers (like NVGT's SDL2 migration)
- **Game Loop** - Unified update/render cycle with proper timing
- **Networking Integration** - Client-server communication for multiplayer
- **Accessibility Layer** - Consistent speech/UI accessibility

---

## Architecture Learning from NVGT

### 1. **Sound Mixer & Effects Chain**

**NVGT Pattern:** Sound objects grouped into mixer chains with effects applied per-group

```cpp
// NVGT-inspired approach
class SoundMixer {
  std::vector<uint32_t> sounds;      // Grouped sounds
  std::vector<Effect> effects;       // Chain: reverb -> distortion -> eq
  float volume;                      // Group volume
  
  void AddSound(uint32_t id);
  void AddEffect(Effect effect);
  void Update();                     // Process effects chain
};

// Usage:
SoundMixer music_mixer;
music_mixer.AddSound(bgm_id);
music_mixer.AddEffect(ReverbEffect{...});
music_mixer.SetVolume(0.8f);
```

**TopSpeed Implementation:**
- Audio sources grouped by gameplay context (music, SFX, UI, dialogue)
- Effect chains: reverb → EQ → compression
- Per-mixer volume independent of source volume
- Real-time effect parameter tweaking

### 2. **Input Handler with Multiple Devices**

**NVGT Pattern:** SDL2 abstraction layer for keyboard/mouse/joystick

```cpp
// NVGT-inspired input system
class InputManager {
  std::map<int, bool> keys_pressed;
  std::map<int, double> axis_values;  // Analog axes
  
  bool IsKeyDown(int keycode);
  bool IsKeyPressed(int keycode);
  double GetAxisValue(int axis);
  
  void Update();
  void HandleEvent(SDL_Event event);
};
```

**TopSpeed Implementation:**
- Configurable key bindings (remappable)
- Axis support for analog controls (steering wheel, pedals)
- Controller dead zones
- Key repeat handling for UI navigation

### 3. **Game Loop with Timing**

**NVGT Pattern:** Fixed-time game loop with frame limiting

```cpp
class GameEngine {
  const float TICK_RATE = 1.0f / 60.0f;  // 60 Hz
  float accumulated_time = 0;
  
  void Run() {
    while (running) {
      float delta = GetDeltaTime();
      accumulated_time += delta;
      
      // Fixed timestep updates
      while (accumulated_time >= TICK_RATE) {
        Update(TICK_RATE);
        accumulated_time -= TICK_RATE;
      }
      
      Render();
      FrameLimit();
    }
  }
};
```

**TopSpeed Implementation:**
- 60 Hz fixed-time updates
- Smooth variable-rate rendering
- Frame time tracking
- Input polling per update

### 4. **Network Integration Pattern**

**NVGT Approach:** Async network I/O with event callbacks

```cpp
class NetworkIntegration {
  // Per-frame network update
  void Update() {
    // Check for new messages
    ProcessIncomingMessages();
    
    // Send queued outgoing messages
    ProcessOutgoingQueue();
    
    // Update connection state
    UpdateConnectionStatus();
  }
  
  // Register callbacks for events
  void OnPlayerJoined(std::function<void(PlayerID)> callback);
  void OnRaceStateChanged(std::function<void(RaceState)> callback);
};
```

**TopSpeed Implementation:**
- Per-frame network polling
- Message queue for batched updates
- Connection state machine
- Event callbacks for game logic

### 5. **Accessibility Integration**

**NVGT Approach:** Consistent screen reader output with optional menu/item announcement

```cpp
class AccessibilitySystem {
  ScreenReaderManager sr;
  
  void AnnounceMenuItem(const std::string& item, int index, int total) {
    sr.SpeakMenuOption(item, index, total);
  }
  
  void AnnounceGameState(const std::string& state) {
    sr.AnnounceEvent(state);
  }
};
```

**TopSpeed Implementation:**
- Menu navigation with item announcement
- Game state changes announced
- Lap times and race progress
- Collision detection audio cues

---

## Phase 3 Implementation Roadmap

### **Week 1: Core Integration**

#### Task 1.1: Sound Mixer System
**Files to create:**
- `src/audio/SoundMixer.h/.cpp` - Mixer with effect chain
- `src/audio/Effects.h/.cpp` - Reverb, EQ, compression
- `src/audio/MixerManager.h/.cpp` - Multi-mixer coordination

**Deliverables:**
- ✅ Sound grouping by context
- ✅ Effect chain processing
- ✅ Per-mixer volume control
- ✅ Real-time parameter tweaking

**Tests:**
- Load 3+ simultaneous audio files
- Apply different effects to different groups
- Verify volume levels independently

#### Task 1.2: Input Handler
**Files to create:**
- `src/input/InputManager.h/.cpp` - SDL3 input abstraction
- `src/input/KeyBindings.h/.cpp` - Configurable bindings
- `src/input/ControllerHandler.h/.cpp` - Joystick/gamepad support

**Deliverables:**
- ✅ Keyboard event handling
- ✅ Key binding system
- ✅ Controller axis support
- ✅ Dead zone handling

**Tests:**
- Verify key press/release events
- Test key binding remapping
- Check analog axis values

#### Task 1.3: Game Loop Architecture
**Files to create:**
- `src/engine/GameEngine.h/.cpp` - Main loop and timing
- `src/engine/GameState.h/.cpp` - State machine
- `src/engine/GameClock.h/.cpp` - Frame timing

**Deliverables:**
- ✅ 60 Hz fixed-time updates
- ✅ Frame limiting
- ✅ Delta time calculation
- ✅ State transitions

**Tests:**
- Verify 60 Hz update frequency
- Check frame time consistency
- Test state transitions

---

### **Week 2: Network & Game Integration**

#### Task 2.1: Network Integration Layer
**Files to create:**
- `src/network/NetworkIntegration.h/.cpp` - High-level API
- `src/network/MessageQueue.h/.cpp` - Message batching
- `src/network/RaceProtocol.h/.cpp` - Game-specific messages

**Deliverables:**
- ✅ Per-frame network update
- ✅ Message queue system
- ✅ Connection state machine
- ✅ Event callbacks

**Tests:**
- Send/receive test messages
- Verify message ordering
- Check connection state transitions

#### Task 2.2: Game Logic Integration
**Files to update:**
- `vs_projects/topspeed/Game.cpp` - Wire all subsystems
- `vs_projects/topspeed/RaceInput.cpp` - Input → game logic
- `vs_projects/topspeed/RaceSettings.cpp` - Config system

**Deliverables:**
- ✅ Game loop running all subsystems
- ✅ Input driving car physics
- ✅ Network updates game state
- ✅ Audio reflecting gameplay

**Tests:**
- Single-player race completion
- Multiplayer race sync
- Audio playing during race

#### Task 2.3: Accessibility Integration
**Files to create:**
- `src/ui/AccessibleMenuSystem.h/.cpp` - Menu with SR support
- `src/ui/RaceUI.h/.cpp` - In-race UI announcements

**Deliverables:**
- ✅ Menu navigation with announcements
- ✅ Race state announcements
- ✅ Lap timing announcements
- ✅ Collision audio feedback

**Tests:**
- Navigate menu with keyboard
- Verify all announcements work
- Check timing feedback

---

### **Week 3: Testing & Optimization**

#### Task 3.1: Unit Testing
**Create test suite:**
- `tests/test_audio_mixer.cpp`
- `tests/test_input_manager.cpp`
- `tests/test_game_loop.cpp`
- `tests/test_network.cpp`

**Coverage:**
- Audio: Mixing, effects, volume
- Input: Key events, bindings, controller axes
- Loop: Timing, state transitions
- Network: Message sending/receiving

#### Task 3.2: Integration Testing
**Create scenarios:**
- `tests/scenario_single_race.cpp` - Single-player race flow
- `tests/scenario_multiplayer_race.cpp` - Network sync
- `tests/scenario_menu_navigation.cpp` - UI accessibility

**Validation:**
- Race completes successfully
- Multiplayer stays in sync
- All accessibility features work

#### Task 3.3: Performance Optimization
**Profile and optimize:**
- Audio mixing performance
- Input polling overhead
- Network message processing
- Memory allocations in hot loop

**Targets:**
- <5ms frame time (60 Hz = 16.6ms budget)
- Audio mixing <1ms per frame
- Network updates <0.5ms per frame
- Zero allocations per frame

---

## Detailed File Structure

```
src/
├── audio/
│   ├── SoundMixer.h/cpp          (NEW - Mixer system)
│   ├── Effects.h/cpp              (NEW - Effect chains)
│   ├── MixerManager.h/cpp         (NEW - Multi-mixer)
│   ├── AudioEngine.h/cpp          (EXISTING)
│   └── VorbisDecoder.h/cpp        (EXISTING)
│
├── input/
│   ├── InputManager.h/cpp         (NEW - SDL3 abstraction)
│   ├── KeyBindings.h/cpp          (NEW - Key remapping)
│   └── ControllerHandler.h/cpp    (NEW - Gamepad support)
│
├── engine/
│   ├── GameEngine.h/cpp           (NEW - Main loop)
│   ├── GameState.h/cpp            (NEW - State machine)
│   ├── GameClock.h/cpp            (NEW - Frame timing)
│   └── GameConfig.h/cpp           (NEW - Settings)
│
├── network/
│   ├── NetworkIntegration.h/cpp   (NEW - High-level API)
│   ├── MessageQueue.h/cpp         (NEW - Message batching)
│   ├── RaceProtocol.h/cpp         (NEW - Game protocol)
│   ├── NetworkManager.h/cpp       (EXISTING)
│   ├── UDPSocket.h/cpp            (EXISTING)
│   └── TCPSocket.h/cpp            (EXISTING)
│
├── ui/
│   ├── AccessibleMenuSystem.h/cpp (NEW - Menu + SR)
│   ├── RaceUI.h/cpp               (NEW - In-race UI)
│   └── UIState.h/cpp              (NEW - UI state machine)
│
└── accessibility/
    └── ScreenReaderManager.h/cpp  (EXISTING)

tests/
├── test_audio_mixer.cpp           (NEW)
├── test_input_manager.cpp         (NEW)
├── test_game_loop.cpp             (NEW)
├── test_network.cpp               (NEW)
├── scenario_single_race.cpp       (NEW)
├── scenario_multiplayer_race.cpp  (NEW)
└── scenario_menu_navigation.cpp   (NEW)
```

---

## Key Design Decisions (NVGT-Inspired)

### 1. **Decoupled Systems**
- Audio, input, network are independent modules
- Game engine coordinates them
- Easy to mock/test individual systems

### 2. **Event-Driven Architecture**
- Input generates events
- Network generates events
- Game state responds to events
- Accessibility observes state changes

### 3. **Fixed Timestep Physics**
- 60 Hz simulation updates
- Deterministic behavior
- Easy to debug and replay
- Network can sync with fewer messages

### 4. **Message Queue for Networking**
- Batch updates per frame
- Reduce network traffic
- Handle packet loss gracefully
- Maintain ordering

### 5. **Accessible by Default**
- All state changes announced
- Menu navigation with screen reader
- No special "accessible mode"
- Same experience for all users

---

## Testing Strategy

### Unit Tests
```cpp
TEST_CASE("Audio Mixer: Add and remove sources") {
  SoundMixer mixer;
  uint32_t id1 = audio.LoadAudio("test1.ogg");
  uint32_t id2 = audio.LoadAudio("test2.ogg");
  
  mixer.AddSound(id1);
  mixer.AddSound(id2);
  
  REQUIRE(mixer.GetSoundCount() == 2);
  
  mixer.RemoveSound(id1);
  REQUIRE(mixer.GetSoundCount() == 1);
}
```

### Integration Tests
```cpp
TEST_CASE("Single race: Start to finish") {
  Game game;
  game.Initialize();
  
  // Menu to race
  game.HandleInput(KeyCode::Enter);  // Start race
  
  // Simulate race
  for (int i = 0; i < 6000; i++) {  // 100 seconds at 60 Hz
    game.Update(1.0f/60.0f);
  }
  
  // Verify race completed
  REQUIRE(game.GetRaceState() == RaceState::Finished);
}
```

---

## Performance Targets

| Component | Budget | Target |
|-----------|--------|--------|
| Game Loop | 16.6ms | <5ms |
| Audio Mixing | 5ms | <1ms |
| Input Processing | 1ms | <0.5ms |
| Network Update | 2ms | <0.5ms |
| Render (when added) | 8ms | <3ms |
| **Total** | **16.6ms** | **<10ms** |

---

## Comparison with NVGT

| Feature | NVGT | TopSpeed |
|---------|------|----------|
| **Sound System** | miniaudio + effects | miniaudio + mixers + effects |
| **Input** | SDL2 abstraction | SDL3 abstraction + controller support |
| **Game Loop** | Fixed timestep | Fixed timestep (60Hz) |
| **Network** | Plugin-based | Native UDP/TCP |
| **Accessibility** | Screen reader + TTS | Screen reader + TTS + menu sr |
| **Language** | AngelScript | C++ |
| **Target** | Audio games | Audio racing game |

---

## Risks & Mitigations

| Risk | Impact | Mitigation |
|------|--------|------------|
| Audio sync drift in multiplayer | Medium | Message timestamps, clock sync |
| Input lag from polling | Low | Higher poll rate (1000Hz) |
| Network packet loss | Medium | Retransmit critical updates |
| Frame timing inconsistency | Medium | Fixed timestep + frame limiter |
| Memory leaks in hot loop | High | Preallocate buffers, no alloc per frame |

---

## Success Criteria

✅ **Phase 3 Complete When:**

1. **Audio System**
   - ✅ Multiple sources playing simultaneously
   - ✅ Per-group volume control
   - ✅ Effects chain working
   - ✅ <1ms per-frame overhead

2. **Input System**
   - ✅ Keyboard input responsive
   - ✅ Key bindings configurable
   - ✅ Controller support working
   - ✅ No input lag detected

3. **Game Loop**
   - ✅ Maintains 60 Hz fixed update rate
   - ✅ Frame timing accurate (±1ms)
   - ✅ All subsystems integrated
   - ✅ <5ms total per-frame overhead

4. **Networking**
   - ✅ Two clients can connect
   - ✅ Race state syncs reliably
   - ✅ Messages sent/received correctly
   - ✅ Handles disconnection gracefully

5. **Accessibility**
   - ✅ Screen reader detects changes
   - ✅ Menu navigation announces items
   - ✅ Race progress announced
   - ✅ All game states accessible

6. **Testing**
   - ✅ Unit tests for all components (>80% coverage)
   - ✅ Integration tests for workflows
   - ✅ Performance benchmarks established
   - ✅ No memory leaks detected

---

## Next Phase: Phase 4

After Phase 3 completion, Phase 4 will focus on:
1. Gameplay refinement
2. Additional race modes
3. User interface polish
4. Documentation and release

---

## References

**NVGT Architecture:**
- [NVGT GitHub](https://github.com/samtupy/nvgt)
- [NVGT Documentation](https://nvgt.dev/docs/)
- Sound mixer pattern
- SDL2 input abstraction
- Fixed-timestep game loop

**TopSpeed Phase 2:**
- Audio Engine (miniaudio)
- Networking (UDP/TCP)
- Screen Reader Support

---

**Status:** Ready to begin Phase 3 implementation  
**Next Step:** Create Sound Mixer system (Task 1.1)
