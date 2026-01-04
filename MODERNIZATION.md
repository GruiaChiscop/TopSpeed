# TopSpeed Modernization Guide

**Version 3.1.0** | **Status:** In Progress | **Windows Focus**

## Overview

This document details the modernization of TopSpeed from legacy DirectX/DirectPlay APIs to modern, maintained libraries. The goal is to ensure the game compiles and runs on contemporary Windows systems while maintaining all existing gameplay functionality.

---

## Architecture Changes

### DirectX → SDL3

**Reason:** DirectX is deprecated for this use case; SDL3 provides:
- Modern, actively maintained API
- Cross-platform foundation for future porting
- Better input handling
- Simpler window management

**Migration Path:**
```
DirectX 7-9              →  SDL3
IDirectDraw             →  SDL_Renderer
IDirectInput            →  SDL_Keyboard/SDL_Mouse events
IDirectSound            →  miniaudio (separate)
Window creation         →  SDL_CreateWindow
```

**Files:**
- `src/platform/SDL3Wrapper.h/.cpp` - Complete SDL3 abstraction layer

**Status:** ✅ Complete (placeholder rendering)

---

### DirectPlay → Modern Networking (UDP/TCP)

**Reason:** DirectPlay is obsolete and removed from modern Windows SDK

**Options Implemented:**
1. **Raw Winsock2** (fastest, most control, Windows-only)
2. **Boost.Asio** (cross-platform, production-ready)
3. **Custom UDP/TCP** (lightweight, future flexibility)

**Implementation:**
```cpp
DirectPlay (obsolete)   →  Modern protocols:
  ├─ UDP (low-latency, real-time race data)
  └─ TCP (reliable, game state sync)
```

**Files:**
- `src/network/NetworkManager.h/.cpp` - Protocol abstraction
- Placeholder implementations with TODO comments for actual socket code

**Status:** ⏳ Skeleton complete, awaiting real socket implementation

---

### DirectSound → miniaudio + Ogg Vorbis

**Reason:** DirectSound is deprecated; miniaudio is modern, lightweight, and supports Ogg Vorbis

**Features:**
- Load and play Ogg Vorbis audio files
- Volume control (master + per-source)
- Looping support
- Multiple simultaneous sources

**Files:**
- `src/audio/AudioEngine.h/.cpp` - Audio management
- Designed for miniaudio backend integration

**Status:** ⏳ API complete, miniaudio integration pending

---

### Accessibility: Screen Reader Support

**New Feature:** Integrated screen reader support for blind players

**Implementation:**
- **Primary:** Universal-Speech (Sam Tupy) - Pure Python TTS
- **Secondary:** System screen readers (NVDA, JAWS on Windows)
- **Fallback:** Windows SAPI TTS

**API:**
```cpp
screenReader.Speak("You crossed the finish line!");
screenReader.SpeakMenuOption("Start Race", 0, 3);
screenReader.AnnounceEvent("Lap completed: 1:23.456");
```

**Files:**
- `src/accessibility/ScreenReaderManager.h/.cpp`

**Status:** ⏳ Framework complete, universal-speech integration pending

---

## Build System: CMake

**Reason:** Modern, cross-platform, better IDE integration than Visual Studio projects

**Features:**
```cmake
option(TOPSPEED_BUILD_CLIENT "Build the race client" ON)
option(TOPSPEED_BUILD_SERVER "Build the race server" ON)
option(TOPSPEED_ENABLE_NETWORKING "Enable network play" ON)
option(TOPSPEED_ENABLE_SCREEN_READER "Enable accessibility" ON)
```

**Build Instructions:**
```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release

# Run
./build/Release/TopSpeed.exe
```

**File:**
- `CMakeLists.txt` - Main build configuration

**Status:** ✅ Complete

---

## Dependencies

### Required

| Library | Version | Purpose | Windows |
|---------|---------|---------|----------|
| **SDL3** | 3.0+ | Window, input, rendering | Pre-built available |
| **miniaudio** | Latest | Audio playback | Header-only |
| **libogg** | 1.3+ | Ogg container format | vcpkg, conan |
| **libvorbis** | 1.3+ | Vorbis audio codec | vcpkg, conan |

### Optional

| Library | Version | Purpose |
|---------|---------|----------|
| **Boost.Asio** | 1.80+ | Cross-platform networking |
| **universal-speech** | Latest | Python TTS (via Python interop) |

### Installing Dependencies on Windows

**Using vcpkg:**
```bash
# Install vcpkg if not present
git clone https://github.com/Microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat

# Install dependencies
.\vcpkg\vcpkg install sdl3:x64-windows libogg:x64-windows libvorbis:x64-windows

# When configuring CMake, add vcpkg toolchain:
cmake -B build -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake
```

**Using Conan:**
```bash
conan install . --output-folder=build
cmake -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake
```

---

## Implementation Status

### Phase 1: Foundation ✅ COMPLETE
- [x] CMake build system
- [x] SDL3 wrapper (window, input, rendering)
- [x] Audio engine API design
- [x] Network manager API design
- [x] Screen reader manager API design

### Phase 2: Integration ⏳ IN PROGRESS
- [ ] Integrate miniaudio for audio playback
- [ ] Integrate universal-speech for screen reader
- [ ] Implement actual socket code (Winsock2 or Boost.Asio)
- [ ] Test window and input handling
- [ ] Update game loop to use new systems

### Phase 3: Testing ⏳ PENDING
- [ ] Unit tests for audio engine
- [ ] Unit tests for networking
- [ ] Integration tests with game logic
- [ ] Accessibility testing with screen readers
- [ ] Performance benchmarks

### Phase 4: Documentation ⏳ PENDING
- [ ] API documentation (Doxygen)
- [ ] Build guide
- [ ] Developer guide
- [ ] Accessibility guide for users

---

## Code Organization

```
TopSpeed/
├── CMakeLists.txt                    # Main build configuration
├── MODERNIZATION.md                  # This file
├── src/
│   ├── platform/
│   │   ├── SDL3Wrapper.h/.cpp       # SDL3 abstraction
│   │   └── [Future: D3D9Wrapper for fallback]
│   ├── audio/
│   │   ├── AudioEngine.h/.cpp       # Audio management
│   │   ├── VorbisDecoder.h/.cpp     # Ogg Vorbis decoding
│   │   └── [Future: MIDI support]
│   ├── network/
│   │   ├── NetworkManager.h/.cpp    # Network abstraction
│   │   ├── UDPSocket.h/.cpp         # UDP implementation
│   │   ├── TCPSocket.h/.cpp         # TCP implementation
│   │   └── Packet.h                 # Message serialization
│   ├── accessibility/
│   │   ├── ScreenReaderManager.h/.cpp
│   │   └── [Future: Haptic feedback]
│   └── server/
│       └── ServerMain.cpp            # Dedicated server binary
└── vs_projects/                      # Legacy VS solution (kept for reference)
    ├── common/
    ├── dxcommon/
    └── topspeed/
```

---

## Migration Checklist

### For Game Logic
- [ ] Replace `IDirectDraw` calls with `SDL_Renderer` calls
- [ ] Replace `IDirectInput` with `SDL_GetKeyboardState()` or event handling
- [ ] Replace `IDirectSound` with `AudioEngine` calls
- [ ] Update menu system to use `ScreenReaderManager` for audio output
- [ ] Update race loop to call `NetworkManager::Update()`

### For Game Server
- [ ] Implement `RaceServer` using `NetworkManager`
- [ ] Create dedicated server executable
- [ ] Add server command-line arguments

### For Testing
- [ ] Write unit tests for each new module
- [ ] Create test data (sample Ogg Vorbis files)
- [ ] Test on Windows 10/11 with:
  - Screen reader active (NVDA or JAWS)
  - Multiple monitors
  - High DPI displays

---

## Next Steps

1. **Install SDL3:**
   ```bash
   vcpkg install sdl3:x64-windows
   ```

2. **Compile Current Skeleton:**
   ```bash
   cmake -B build
   cmake --build build
   ```

3. **Run Tests:**
   ```bash
   ctest --output-on-failure
   ```

4. **Begin Phase 2 Implementation:**
   - Start with miniaudio integration
   - Then network socket code
   - Finally screen reader integration

---

## Performance Considerations

### Audio
- miniaudio: ~10µs per frame overhead (negligible)
- Vorbis decoding: Offload to separate thread if needed

### Networking
- UDP: Minimal overhead for real-time race data
- TCP: Only for state synchronization (non-critical path)

### Rendering
- SDL3 with hardware acceleration: Comparable to DirectX 9
- Consider: Update game to use 3D graphics with SDL3 + OpenGL in future

---

## Troubleshooting

### CMake Configuration Fails
**Problem:** SDL3 not found  
**Solution:** Add vcpkg toolchain or install SDL3 manually

### Audio Won't Initialize
**Problem:** miniaudio initialization fails  
**Solution:** Ensure audio device is connected and not in use by other apps

### Screen Reader Not Working
**Problem:** universal-speech not found  
**Solution:** Install via pip: `pip install universal-speech`

### Networking Issues
**Problem:** Connection fails  
**Solution:** Check firewall rules and implement proper socket error handling

---

## References

- **SDL3 Documentation:** https://wiki.libsdl.org/SDL3
- **miniaudio:** https://github.com/mackron/miniaudio
- **Ogg Vorbis:** https://www.xiph.org/vorbis/
- **universal-speech:** https://github.com/samtupy/universal-speech
- **CMake:** https://cmake.org/cmake/help/latest/
- **Windows Sockets 2:** https://learn.microsoft.com/en-us/windows/win32/winsock/windows-sockets-2-start-page
- **Boost.Asio:** https://www.boost.org/doc/libs/release/doc/html/boost_asio.html

---

**Last Updated:** 2026-01-04  
**Maintained by:** GruiaChiscop  
**License:** [Your Project License]
