# Phase 2: Build & Test Instructions

## Quick Build

### Step 1: Prepare Environment

```bash
# On Windows, from TopSpeed directory
cd TopSpeed
git checkout modernize/sdl3-miniaudio

# Install/verify vcpkg dependencies
if not exist vcpkg (
    git clone https://github.com/Microsoft/vcpkg.git
    .\vcpkg\bootstrap-vcpkg.bat
)

# Install libraries
.\vcpkg\vcpkg install sdl3:x64-windows libogg:x64-windows libvorbis:x64-windows
```

### Step 2: Configure CMake

```bash
mkdir build
cd build

# Configure with vcpkg toolchain
cmake .. -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
```

### Step 3: Build

```bash
# Build all targets
cmake --build . --config Release

# Or verbose output
cmake --build . --config Release --verbose
```

### Expected Output

```
TopSpeed 3.1.0
C++ Standard: 17
Build Client: ON
Build Server: ON
Enable Networking: ON
Enable Screen Reader: ON

-- Downloading miniaudio...
-- Building libraries...
-- Built target TopSpeedCommon
-- Built target DXCommon  
-- Built target TopSpeedAudio
-- Built target TopSpeedScreenReader
-- Built target TopSpeedNetworking
-- Built target TopSpeed
-- Built target TopSpeedServer
```

---

## Verification

### Build Artifacts

```bash
# Check build output
dir build\Release\*

# Should see:
# TopSpeed.exe          (Main game client)
# TopSpeedServer.exe    (Dedicated server)
# *.lib files           (Static libraries)
```

### Verify Dependencies Downloaded

```bash
# Check miniaudio
dir build\_deps\miniaudio-src\

# Should contain: miniaudio.h
type build\_deps\miniaudio-src\miniaudio.h
```

---

## Testing Audio Engine

### Create Test Audio File

1. Generate a test Ogg Vorbis file (or download sample)
2. Place in `data/audio/test.ogg`

### Test Code

```cpp
#include "audio/AudioEngine.h"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    TopSpeed::AudioEngine audio;
    
    std::cout << "=== Audio Engine Test ===" << std::endl;
    
    if (!audio.Initialize()) {
        std::cerr << "Failed to initialize audio" << std::endl;
        return 1;
    }
    std::cout << "✓ Audio initialized" << std::endl;
    
    // Load audio
    uint32_t id = audio.LoadAudio("data/audio/test.ogg");
    if (id == 0) {
        std::cerr << "Failed to load audio" << std::endl;
        return 1;
    }
    std::cout << "✓ Audio loaded (ID: " << id << ")" << std::endl;
    
    // Play audio
    audio.Play(id, false);  // No loop
    std::cout << "✓ Audio playing" << std::endl;
    
    // Wait for playback
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // Update engine
    audio.Update();
    
    // Stop and cleanup
    audio.Stop(id);
    audio.Unload(id);
    audio.Shutdown();
    
    std::cout << "✓ Audio test complete" << std::endl;
    return 0;
}
```

---

## Testing Networking

### Test UDP

```cpp
#include "network/NetworkManager.h"
#include <thread>
#include <chrono>

int main() {
    std::cout << "=== Network UDP Test ===" << std::endl;
    
    // Create server
    TopSpeed::NetworkManager server;
    if (!server.Initialize()) {
        std::cerr << "Server init failed" << std::endl;
        return 1;
    }
    
    if (!server.Listen(9999, TopSpeed::NetworkManager::ProtocolType::UDP)) {
        std::cerr << "Server listen failed" << std::endl;
        return 1;
    }
    std::cout << "✓ UDP server listening on :9999" << std::endl;
    
    // Create client
    TopSpeed::NetworkManager client;
    if (!client.Initialize()) {
        std::cerr << "Client init failed" << std::endl;
        return 1;
    }
    
    // Send packet
    const char* msg = "Hello UDP!";
    if (client.SendTo("127.0.0.1", 9999, msg, strlen(msg))) {
        std::cout << "✓ UDP packet sent" << std::endl;
    }
    
    // Wait and receive
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    char buffer[256];
    size_t bytes = server.Receive(buffer, sizeof(buffer));
    if (bytes > 0) {
        std::cout << "✓ UDP packet received: " << std::string(buffer, bytes) << std::endl;
    }
    
    server.Shutdown();
    client.Shutdown();
    return 0;
}
```

### Test TCP

```cpp
#include "network/NetworkManager.h"
#include <thread>

int main() {
    std::cout << "=== Network TCP Test ===" << std::endl;
    
    // Server thread
    std::thread server_thread([]() {
        TopSpeed::NetworkManager server;
        server.Initialize();
        server.Listen(9998, TopSpeed::NetworkManager::ProtocolType::TCP);
        std::cout << "✓ TCP server listening on :9998" << std::endl;
        
        // Wait for data
        char buffer[256];
        size_t bytes = server.Receive(buffer, sizeof(buffer));
        if (bytes > 0) {
            std::cout << "✓ TCP received: " << std::string(buffer, bytes) << std::endl;
        }
        
        server.Shutdown();
    });
    
    // Give server time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Client
    TopSpeed::NetworkManager client;
    client.Initialize();
    
    if (client.Connect("127.0.0.1", 9998, TopSpeed::NetworkManager::ProtocolType::TCP)) {
        std::cout << "✓ TCP client connected" << std::endl;
        
        const char* msg = "Hello TCP!";
        if (client.Send(msg, strlen(msg))) {
            std::cout << "✓ TCP data sent" << std::endl;
        }
    }
    
    client.Shutdown();
    server_thread.join();
    return 0;
}
```

---

## Testing Screen Reader

### Test Code

```cpp
#include "accessibility/ScreenReaderManager.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "=== Screen Reader Test ===" << std::endl;
    
    TopSpeed::ScreenReaderManager sr;
    
    if (!sr.Initialize()) {
        std::cerr << "Failed to initialize screen reader" << std::endl;
        return 1;
    }
    std::cout << "✓ Screen reader initialized" << std::endl;
    
    std::cout << "Screen Reader Available: " << (sr.IsScreenReaderAvailable() ? "YES" : "NO") << std::endl;
    std::cout << "TTS Available: " << (sr.IsTTSAvailable() ? "YES" : "NO") << std::endl;
    
    // Test speaking
    sr.Speak("Welcome to TopSpeed audio game.");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Test menu announcements
    sr.SpeakMenuOption("Start New Race", 0, 3);
    sr.SpeakMenuOption("Continue", 1, 3);
    sr.SpeakMenuOption("Quit", 2, 3);
    
    // Test event announcements
    sr.AnnounceEvent("Race started!");
    sr.AnnounceEvent("Lap 1 complete. Time: 1 minute 23 seconds.");
    sr.AnnounceEvent("You finished first!");
    
    sr.Stop();
    sr.Shutdown();
    
    std::cout << "✓ Screen reader test complete" << std::endl;
    return 0;
}
```

---

## Integration Test

Create `tests/test_phase2.cpp`:

```cpp
#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include "audio/AudioEngine.h"
#include "network/NetworkManager.h"
#include "accessibility/ScreenReaderManager.h"

TEST_CASE("Audio Engine Initialization") {
    TopSpeed::AudioEngine audio;
    REQUIRE(audio.Initialize() == true);
    audio.Shutdown();
}

TEST_CASE("Network Manager Initialization") {
    TopSpeed::NetworkManager net;
    REQUIRE(net.Initialize() == true);
    net.Shutdown();
}

TEST_CASE("Screen Reader Initialization") {
    TopSpeed::ScreenReaderManager sr;
    REQUIRE(sr.Initialize() == true);
    REQUIRE(sr.IsEnabled() == true);
    sr.Shutdown();
}
```

---

## Troubleshooting

### Build Fails: "SDL3 not found"

**Solution:**
```bash
.\vcpkg\vcpkg install sdl3:x64-windows --force
cmake .. -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake
```

### Build Fails: "vorbis/vorbisfile.h not found"

**Solution:**
```bash
.\vcpkg\vcpkg install libvorbis:x64-windows --force
```

### Runtime: DLL not found

**Solution 1: Copy DLLs**
```bash
copy .\vcpkg\installed\x64-windows\bin\*.dll .\build\Release\
```

**Solution 2: Update PATH**
```bash
set PATH=%PATH%;C:\path\to\TopSpeed\vcpkg\installed\x64-windows\bin
```

### Audio Test: "Failed to load audio"

**Solution:**
1. Verify test file exists: `data/audio/test.ogg`
2. Check file is valid Ogg Vorbis format
3. Verify path is relative to executable directory

### Network Test: "WSA Error: 10048"

**Solution:** Port already in use
```bash
# Find and kill process using port 9999
netstat -ano | findstr :9999
taskkill /PID <PID> /F
```

---

## Performance Baseline

### Expected Performance

**Audio Engine:**
- Initialization: <100ms
- Load Ogg file: 50-200ms (depends on file size)
- Audio mixing: 10-50µs per frame (at 60 FPS)
- Memory: ~1MB per 10 seconds of audio

**Networking:**
- UDP packet: <1ms roundtrip (local network)
- TCP connection: <5ms (local network)
- Receive thread: <1% CPU overhead
- Memory: <1MB for connection

**Screen Reader:**
- Initialization: <50ms
- Speech output: <10ms (console only)
- Memory: <1MB

---

## Next Steps

1. ✅ Build successfully
2. ✅ Run tests (pass all)
3. ✅ Verify performance baseline
4. ✅ Check console output
5. ✓ Ready for Phase 3: Game Loop Integration

---

## Support

**Build issues?** Check BUILD_SETUP.md  
**API questions?** See PHASE2_COMPLETE.md  
**Architecture details?** Read MODERNIZATION.md

---

**Ready to build?**
```bash
cmake --build build --config Release
.\build\Release\TopSpeed.exe
```

Good luck! 🚀
