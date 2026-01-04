# TopSpeed Modernization Branch

**Branch:** `modernize/sdl3-miniaudio`  
**Status:** 🖄 Under Active Development  
**Phase:** 1/4 Complete (Foundation & APIs)  
**Target Release:** v3.1.0

---

## What Is This Branch?

This is the modernization work for TopSpeed audio racing game, migrating from deprecated DirectX 7-9 and DirectPlay APIs to modern, maintained libraries.

**Main Goal:** Get the game running on Windows 10/11 with contemporary development tools, then gradually enhance features.

---

## For First-Time Visitors

### 🪨 Quick Navigation

1. **First time here?** Start with [MODERNIZATION_SUMMARY.md](MODERNIZATION_SUMMARY.md)
2. **Want to build?** Go to [BUILD_SETUP.md](BUILD_SETUP.md)
3. **Need details?** Read [MODERNIZATION.md](MODERNIZATION.md)
4. **Want to contribute?** See [CONTRIBUTING.md](#contributing) below

### ✱ In 30 Seconds

- **Old:** DirectX 7-9, DirectPlay, DirectSound
- **New:** SDL3, Modern UDP/TCP, miniaudio + Ogg Vorbis
- **Status:** Build system + APIs done, integration in progress
- **You Can:** Build the skeleton right now!

---

## Project Structure

```
topspeed-modernize/
├── 📚 Documentation
│   ├── MODERNIZATION.md              ← Read this for deep dive
│   ├── MODERNIZATION_SUMMARY.md       ← Quick reference
│   ├─┠── BUILD_SETUP.md                ← How to build
│   └── README_MODERNIZATION.md (you are here)
│
├── 📄 Build Configuration
│   ├┠── CMakeLists.txt                ✅ Complete
│   └── vcpkg.json (future)           ⏳ Planned
│
├── 💾 Source Code (Phase 1: Complete)
│   ├── src/platform/
│   │   ├┠── SDL3Wrapper.h               ✅ Implemented
│   │   └── SDL3Wrapper.cpp             ✅ Implemented
│   ├── src/audio/
│   │   ├┠── AudioEngine.h               ✅ Implemented (placeholder)
│   │   └── AudioEngine.cpp             ✅ Implemented (placeholder)
│   ├── src/network/
│   │   ├┠── NetworkManager.h            ✅ Implemented (placeholder)
│   │   └── NetworkManager.cpp          ✅ Implemented (placeholder)
│   └── src/accessibility/
│       ├┠── ScreenReaderManager.h       ✅ Implemented (placeholder)
│       └── ScreenReaderManager.cpp     ✅ Implemented (placeholder)
│
├── 🐵 Legacy Code (Preserved)
│   └── vs_projects/                  Keep for reference
│       ├── common/
│       ├── dxcommon/
│       └── topspeed/
│
└── 🪜 Tests (Future)
    └── tests/                        ⏳ Planned for Phase 3
```

---

## Current Status

### Phase 1: Foundation ✅ COMPLETE

- [x] CMake build system for Windows
- [x] SDL3 window/input wrapper
- [x] Audio engine API (miniaudio-ready)
- [x] Networking API (UDP/TCP)
- [x] Screen reader accessibility API
- [x] Comprehensive documentation
- [x] Build setup guide

**Deliverables:**
- Compiles cleanly on Windows 10/11 with Visual Studio 2022
- All APIs documented and placeholder-implemented
- CMake configuration ready for dependency integration

### Phase 2: Integration ⏳ IN PROGRESS

- [ ] Integrate miniaudio for audio
- [ ] Implement actual socket code
- [ ] Wire game loop to SDL3Wrapper
- [ ] Test audio playback
- [ ] Test network connectivity
- [ ] Accessibility testing

**Estimated:** 2-3 weeks

### Phase 3: Testing ⏳ PENDING

- [ ] Unit tests for each module
- [ ] Integration tests
- [ ] Performance profiling
- [ ] Bug fixes and optimization

**Estimated:** 2 weeks

### Phase 4: Polish ⏳ PENDING

- [ ] Full documentation
- [ ] Example projects
- [ ] Binary distribution setup
- [ ] Release notes

**Estimated:** 1 week

---

## Technology Stack

| Component | Technology | Why? |
|-----------|-----------|------|
| **Windowing** | SDL3 | Modern, cross-platform, actively maintained |
| **Rendering** | SDL_Renderer | Hardware accelerated, integrated with SDL |
| **Input** | SDL3 Events | Better than DirectInput |
| **Audio** | miniaudio + Ogg Vorbis | Lightweight, excellent Vorbis support |
| **Networking** | Modern UDP/TCP | Replaces deprecated DirectPlay |
| **Accessibility** | universal-speech | Cross-platform TTS fallback |
| **Build** | CMake 3.20+ | Modern, IDE integration |
| **IDE** | Visual Studio 2022 | Primary development environment |
| **Language** | C++17 | Modern standard library |

---

## How to Get Started

### Quick Start (15 minutes)

```bash
# 1. Clone and switch branch
git clone https://github.com/GruiaChiscop/TopSpeed.git
cd TopSpeed
git checkout modernize/sdl3-miniaudio

# 2. Install dependencies
git clone https://github.com/Microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg install sdl3:x64-windows libogg:x64-windows libvorbis:x64-windows

# 3. Build
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build . --config Release

# 4. Run
.\Release\TopSpeed.exe
```

**Issues?** Check [BUILD_SETUP.md](BUILD_SETUP.md) for troubleshooting.

---

## Key Files to Review

### 📊 Documentation

- **[MODERNIZATION_SUMMARY.md](MODERNIZATION_SUMMARY.md)** - Executive summary (start here!)
- **[MODERNIZATION.md](MODERNIZATION.md)** - Detailed architecture and design
- **[BUILD_SETUP.md](BUILD_SETUP.md)** - Step-by-step build instructions

### 💾 Code Examples

**SDL3Wrapper Usage:**
```cpp
#include "platform/SDL3Wrapper.h"

TopSpeed::SDL3Wrapper window;
window.Initialize();
window.CreateWindow(800, 600, "TopSpeed");

while (window.ProcessEvents()) {
    window.ClearScreen(0, 0, 0);
    
    if (window.IsKeyPressed(SDL_SCANCODE_UP)) {
        // Handle input
    }
    
    window.Present();
}
```

**AudioEngine Usage:**
```cpp
#include "audio/AudioEngine.h"

TopSpeed::AudioEngine audio;
audio.Initialize();

uint32_t bgm = audio.LoadAudio("music.ogg");
audio.Play(bgm, true);  // Loop background music
audio.SetVolume(bgm, 0.8f);
```

**NetworkManager Usage:**
```cpp
#include "network/NetworkManager.h"

TopSpeed::NetworkManager network;
network.Initialize();
network.Connect("192.168.1.100", 9999, NetworkManager::ProtocolType::UDP);
network.Send(data, size);
```

---

## Contributing

### Want to Help?

1. **Pick a task:**
   - Implement miniaudio integration
   - Implement socket networking
   - Add screen reader support
   - Write unit tests
   - Improve documentation

2. **Create a branch:**
   ```bash
   git checkout -b feature/your-feature
   ```

3. **Make changes:**
   - Follow code style (C++17, modern practices)
   - Add comments for complex logic
   - Test thoroughly

4. **Submit PR:**
   - Clear description of changes
   - Test results
   - Screenshots if applicable

### Code Style

- Use C++17 standard library features
- RAII for resource management
- Snake_case for variables, CamelCase for classes
- Document all public methods
- Use meaningful variable names

### Testing

```bash
# Build
cmake --build build --config Release

# Run tests
ctest --output-on-failure

# Manual testing
.\build\Release\TopSpeed.exe
```

---

## Known Issues

### Phase 1 (Current)

⚠️ **Expected Limitations:**
- Rendering not wired to game loop yet
- Audio engine not fully integrated with miniaudio
- Networking is placeholder implementation
- Screen reader is stub implementation
- No actual game functionality yet

✅ **These are intentional** - Phase 1 is about foundation and APIs.

### Troubleshooting

**Build fails with "SDL3 not found"**
- Ensure vcpkg installed SDL3
- Pass correct toolchain: `-DCMAKE_TOOLCHAIN_FILE=...\vcpkg\...`

**Runtime: "SDL3.dll not found"**
- Copy DLL to executable directory or add to PATH
- See [BUILD_SETUP.md](BUILD_SETUP.md) for details

**CMake complains about compiler**
- Ensure Visual Studio C++ tools installed
- Try: `cmake --version` to verify installation

---

## Performance

### Expected Performance Characteristics

| System | Component | Performance |
|--------|-----------|-------------|
| **CPU** | Rendering | Similar to DirectX 9 |
| **CPU** | Audio | ~10μs/frame (negligible) |
| **CPU** | Networking | Network-bound, not CPU-bound |
| **RAM** | Binary size | ~5-10 MB (estimated) |
| **GPU** | Rendering | Hardware accelerated (SDL3) |

### Optimization Opportunities

1. Async audio decoding (separate thread)
2. Async network I/O (Boost.Asio)
3. GPU particle effects (future)
4. Shader compilation caching

---

## Questions?

### Documentation
1. **What changed?** → [MODERNIZATION_SUMMARY.md](MODERNIZATION_SUMMARY.md)
2. **How do I build?** → [BUILD_SETUP.md](BUILD_SETUP.md)
3. **Why these choices?** → [MODERNIZATION.md](MODERNIZATION.md)
4. **API details?** → Code comments and Doxygen

### Issues
1. Open GitHub issue with:
   - Windows version
   - Visual Studio version
   - Build command used
   - Full error output

### Contributing
1. Review [CONTRIBUTING.md](CONTRIBUTING.md) (when created)
2. Check existing issues for what's needed
3. Submit PR with clear description

---

## Roadmap

### v3.1.0 (Modernization)
- [x] Phase 1: Foundation APIs
- [ ] Phase 2: Integration & Testing
- [ ] Phase 3: Unit & Integration Tests
- [ ] Phase 4: Polish & Documentation

### v3.2.0 (Enhancement)
- [ ] 3D graphics with SDL3 + OpenGL
- [ ] Advanced networking (TCP relay server)
- [ ] Cross-platform build (Mac, Linux)
- [ ] Web version (WebAssembly)

### v3.3.0 (Future)
- [ ] Mobile ports (iOS, Android)
- [ ] VR support
- [ ] Multiplayer tournaments
- [ ] Replay system

---

## Resources

### External Documentation
- [SDL3 Docs](https://wiki.libsdl.org/SDL3)
- [miniaudio](https://github.com/mackron/miniaudio)
- [Ogg Vorbis](https://xiph.org/vorbis/)
- [CMake](https://cmake.org/)
- [universal-speech](https://github.com/samtupy/universal-speech)

### Related Projects
- Original TopSpeed: https://github.com/GruiaChiscop/TopSpeed
- SDL3 Examples: https://github.com/libsdl-org/SDL/tree/main/examples
- miniaudio Examples: https://github.com/mackron/miniaudio/tree/master/examples

---

## License

TopSpeed is licensed under [Your License]. See LICENSE file in root.

---

**Last Updated:** 2026-01-04  
**Maintained by:** GruiaChiscop  
**Status:** 🖄 Active Development  
**Contributing:** Yes, please! See [CONTRIBUTING.md](CONTRIBUTING.md)
