# TopSpeed Build Setup Guide (Windows)

## Prerequisites

- **Windows 10/11** (64-bit)
- **Visual Studio 2022** (Community, Professional, or Enterprise)
  - C++ development tools
  - Windows SDK (included with VS)
- **CMake 3.20+**
- **Git**
- **vcpkg** (recommended for dependency management)

---

## Step 1: Clone Repository

```bash
git clone https://github.com/GruiaChiscop/TopSpeed.git
cd TopSpeed
git checkout modernize/sdl3-miniaudio
```

---

## Step 2: Install Dependencies

### Option A: Using vcpkg (Recommended)

```bash
# Clone vcpkg if not already installed
git clone https://github.com/Microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat

# Install required libraries
.\vcpkg\vcpkg install sdl3:x64-windows libogg:x64-windows libvorbis:x64-windows

# Note the integration path - you'll need it for CMake
echo %cd%\vcpkg
```

**Integration with CMake:**

When configuring CMake, pass the vcpkg toolchain:
```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=.\vcpkg\scripts\buildsystems\vcpkg.cmake
```

### Option B: Manual Installation

1. **Download SDL3:**
   - Visit: https://github.com/libsdl-org/SDL/releases
   - Download `SDL3-3.x.x-win64-devel.zip`
   - Extract to `C:\SDL3`

2. **Download libogg and libvorbis:**
   - Visit: https://xiph.org/vorbis/download/
   - Build from source or download pre-built binaries
   - Extract to `C:\libogg` and `C:\libvorbis`

3. **Update CMakeLists.txt:**
   ```cmake
   set(SDL3_DIR "C:/SDL3")
   set(OGG_DIR "C:/libogg")
   set(VORBIS_DIR "C:/libvorbis")
   ```

---

## Step 3: Configure CMake

### Using vcpkg (Recommended):

```bash
# Create build directory
mkdir build
cd build

# Configure with vcpkg
cmake .. -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_BUILD_TYPE=Release

cd ..
```

### Using Visual Studio IDE:

1. Open Visual Studio
2. File → Open → Folder
3. Select TopSpeed directory
4. CMake will auto-detect configuration
5. Select configuration from dropdown:
   - `x64-Debug`
   - `x64-Release` (recommended)

### Manual Configuration:

```bash
mkdir build
cd build

# Minimal configuration (if paths are in standard locations)
cmake .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022" -A x64

cd ..
```

---

## Step 4: Build

### Command Line:

```bash
cmake --build build --config Release

# Or with verbose output:
cmake --build build --config Release --verbose
```

### Visual Studio IDE:

1. Build → Build Solution (Ctrl+Shift+B)
2. Or select specific target and build

### If Build Fails:

**Check CMake output for missing dependencies:**
```bash
cd build
cmake .. --debug-output
```

**Common issues:**
- `Could NOT find SDL3` → Install SDL3 or pass SDL3_DIR
- `Could NOT find OGG` → Install libogg or check path
- `MSVC compiler not found` → Install Visual Studio C++ tools

---

## Step 5: Run Executable

### Built Executable Locations:

**Release build:**
```
build/Release/TopSpeed.exe          # Client game
build/Release/TopSpeedServer.exe    # Dedicated server (when implemented)
```

**Debug build:**
```
build/Debug/TopSpeed.exe
build/Debug/TopSpeedServer.exe
```

### Running from Command Line:

```bash
# Run game client
.\build\Release\TopSpeed.exe

# Run with additional logging
.\build\Release\TopSpeed.exe --verbose

# Run server (when implemented)
.\build\Release\TopSpeedServer.exe --port 9999
```

### Running from Visual Studio IDE:

1. Select TopSpeed project in Solution Explorer
2. Right-click → Set as Startup Project
3. Debug → Start Debugging (F5) or Start Without Debugging (Ctrl+F5)

---

## Step 6: Run Tests

```bash
# From build directory
ctest --output-on-failure

# Or from project root
cmake --build build --target RUN_TESTS
```

---

## Advanced Configuration Options

### CMake Build Options:

```bash
# Disable networking (for testing core game)
cmake .. -DTOPSPEED_ENABLE_NETWORKING=OFF

# Build server only
cmake .. -DTOPSPEED_BUILD_CLIENT=OFF -DTOPSPEED_BUILD_SERVER=ON

# Disable accessibility features (for lighter build)
cmake .. -DTOPSPEED_ENABLE_SCREEN_READER=OFF

# Debug symbols in Release build (helpful for crash analysis)
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

### Compiler Options:

```bash
# Enable all warnings
cmake .. -DCMAKE_CXX_FLAGS="/W4"

# Link-time optimization (slower build, faster executable)
cmake .. -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON

# 32-bit build (if needed)
cmake .. -A Win32
```

---

## Troubleshooting

### Issue: CMake Configuration Fails

**Error:** `Could not find a package configuration file provided by SDL3`

**Solutions:**
1. Verify SDL3 is installed via vcpkg:
   ```bash
   .\vcpkg\vcpkg list | findstr SDL3
   ```

2. Pass vcpkg toolchain explicitly:
   ```bash
   cmake .. -DCMAKE_TOOLCHAIN_FILE=.\vcpkg\scripts\buildsystems\vcpkg.cmake
   ```

3. Manually specify SDL3_DIR:
   ```bash
   cmake .. -DSDL3_DIR=C:\SDL3\cmake
   ```

### Issue: Linker Errors

**Error:** `LNK1104: cannot open file 'SDL3.lib'`

**Solutions:**
1. Ensure dependencies are installed:
   ```bash
   .\vcpkg\vcpkg list
   ```

2. Rebuild vcpkg packages:
   ```bash
   .\vcpkg\vcpkg remove sdl3:x64-windows
   .\vcpkg\vcpkg install sdl3:x64-windows
   ```

3. Clean and reconfigure:
   ```bash
   rm -r build
   mkdir build
   cd build
   cmake ..
   ```

### Issue: Runtime Error - SDL3 DLL Not Found

**Error:** `The code execution cannot proceed because SDL3.dll was not found`

**Solutions:**
1. Copy DLL to executable directory:
   ```bash
   copy vcpkg\installed\x64-windows\bin\SDL3.dll build\Release\
   copy vcpkg\installed\x64-windows\bin\ogg.dll build\Release\
   copy vcpkg\installed\x64-windows\bin\vorbis.dll build\Release\
   ```

2. Or add vcpkg bin directory to PATH:
   ```bash
   set PATH=%PATH%;C:\path\to\vcpkg\installed\x64-windows\bin
   ```

3. Or use static linking (modify CMakeLists.txt):
   ```cmake
   target_link_options(TopSpeed PRIVATE /SUBSYSTEM:CONSOLE)
   ```

### Issue: High DPI Scaling Issues

**Problem:** Game window appears blurry on 4K monitors

**Solution:** Add to SDL3Wrapper.cpp initialization:
```cpp
SDL_SetHint(SDL_HINT_WINDOWS_DPI_SCALING, "1");
```

### Issue: Screen Reader Not Working

**Problem:** ScreenReaderManager initializes but produces no audio

**Solutions:**
1. Install universal-speech:
   ```bash
   pip install universal-speech
   ```

2. Verify system audio output is working

3. Check Windows accessibility settings are enabled

---

## Development Workflow

### Edit-Compile-Test Cycle:

```bash
# 1. Edit source files
code src/platform/SDL3Wrapper.cpp

# 2. Rebuild (incremental compilation)
cmake --build build --config Release

# 3. Run and test
.\build\Release\TopSpeed.exe
```

### Using Visual Studio for Debugging:

1. Set breakpoint (Click on line number)
2. Press F5 (Debug) or F10 (Debug Step)
3. View variables in Debug window
4. Use Debug Console for commands

### Unit Testing:

```bash
# Run all tests with output
ctest --build-dir build --output-on-failure

# Run specific test
ctest --build-dir build -R AudioEngine --verbose
```

---

## Performance Profiling

### Using Visual Studio Profiler:

1. Debug → Performance Profiler (Alt+F2)
2. Select profiling tools:
   - CPU Usage
   - Memory Usage (for memory leaks)
   - GPU Usage (if applicable)
3. Start collection
4. Run game and interact
5. Stop collection and analyze results

### Command-line Performance:

```bash
# Enable debug output with timing
cmake --build build --config Release --verbose | findstr /C:"time"

# Profile executable runtime
time .\build\Release\TopSpeed.exe
```

---

## Deployment

### Create Distributable Package:

```bash
# Create release directory
mkdir TopSpeed_Release
cd TopSpeed_Release

# Copy executable
copy ..\build\Release\TopSpeed.exe .

# Copy required DLLs
copy ..\vcpkg\installed\x64-windows\bin\SDL3.dll .
copy ..\vcpkg\installed\x64-windows\bin\ogg.dll .
copy ..\vcpkg\installed\x64-windows\bin\vorbis.dll .

# Copy game data (when applicable)
xcopy ..\data . /E

# Create readme
echo "TopSpeed v3.1.0" > README.txt
echo "Run TopSpeed.exe to start" >> README.txt
```

---

## Next Steps

1. **Verify build succeeds:**
   ```bash
   cmake --build build --config Release
   ```

2. **Run executable:**
   ```bash
   .\build\Release\TopSpeed.exe
   ```

3. **Begin Phase 2 implementation:**
   - Integrate miniaudio for audio
   - Implement actual networking code
   - Test screen reader integration

---

## Support

For issues:
1. Check [MODERNIZATION.md](./MODERNIZATION.md) for architectural details
2. Review CMake error messages carefully
3. Open issue on GitHub with:
   - Windows version
   - Visual Studio version
   - CMake configuration command
   - Full error output

---

**Last Updated:** 2026-01-04  
**Tested On:** Windows 10/11 with Visual Studio 2022  
**Maintained by:** GruiaChiscop
