# 🎵 Audio Setup Guide - TopSpeed Racing Game

## 📁 Audio Directory Structure

Create this folder structure in your project root:

```
TopSpeed/
├── assets/
│   └── audio/
│       ├── music/
│       │   ├── menu.ogg          # Main menu background music
│       │   ├── racing.ogg        # In-game racing music
│       │   └── victory.ogg       # Race completion music
│       ├── engine/
│       │   ├── idle.ogg          # Engine idle sound (0-10 km/h)
│       │   ├── low.ogg           # Low RPM (10-50 km/h)
│       │   ├── mid.ogg           # Mid RPM (50-150 km/h)
│       │   ├── high.ogg          # High RPM (150-250 km/h)
│       │   └── max.ogg           # Max RPM (250+ km/h)
│       └── sfx/
│           ├── shift_up.ogg      # Manual transmission shift up
│           ├── shift_down.ogg    # Manual transmission shift down
│           ├── collision.ogg     # Vehicle collision
│           ├── lap_complete.ogg  # Lap completion sound
│           ├── race_start.ogg    # Race start signal
│           ├── race_finish.ogg   # Race finish sound
│           ├── horn.ogg          # Vehicle horn
│           └── damage.ogg        # Damage/crash sound
└── src/
    └── ...
```

---

## 🎵 Audio File Specifications

### Format Requirements

| Property | Requirement | Notes |
|----------|-------------|-------|
| **Format** | OGG Vorbis | Compressed, high quality |
| **Sample Rate** | 44100 Hz | Standard CD quality |
| **Bit Depth** | 16-bit | Stereo recommended |
| **Channels** | Mono or Stereo | Stereo for music, mono for SFX |
| **Bitrate** | 128-256 kbps | Good balance of quality/size |
| **Duration** | Variable | See below |

### Audio Files Details

#### Music Files

**menu.ogg** (Main Menu)
- Duration: 60-120 seconds (loops)
- Volume: Moderate, atmospheric
- Type: Background music
- Loop: YES

**racing.ogg** (In-Game Racing)
- Duration: 60-180 seconds (loops)
- Volume: Energetic, driving
- Type: Background music
- Loop: YES

**victory.ogg** (Race Complete)
- Duration: 15-30 seconds (plays once)
- Volume: Celebratory
- Type: Completion sound
- Loop: NO

#### Engine Sounds

**idle.ogg** (Idle/Very Slow)
- Speed: 0-10 km/h
- Duration: 1-2 seconds
- Pitch: Low (500-800 Hz)
- Type: Looped engine sound

**low.ogg** (Low Speed)
- Speed: 10-50 km/h
- Duration: 1-2 seconds
- Pitch: Low-Medium (800-1200 Hz)
- Type: Looped engine sound

**mid.ogg** (Medium Speed)
- Speed: 50-150 km/h
- Duration: 1-2 seconds
- Pitch: Medium (1200-1800 Hz)
- Type: Looped engine sound

**high.ogg** (High Speed)
- Speed: 150-250 km/h
- Duration: 1-2 seconds
- Pitch: High (1800-2500 Hz)
- Type: Looped engine sound

**max.ogg** (Maximum Speed)
- Speed: 250+ km/h
- Duration: 1-2 seconds
- Pitch: Very High (2500+ Hz)
- Type: Looped engine sound

#### SFX Files

**shift_up.ogg**
- Duration: 0.2-0.5 seconds
- Type: Short beep/click
- Usage: Manual transmission upshift

**shift_down.ogg**
- Duration: 0.2-0.5 seconds
- Type: Short beep/click
- Usage: Manual transmission downshift

**collision.ogg**
- Duration: 0.5-1.0 seconds
- Type: Impact sound
- Usage: Vehicle-to-vehicle collision

**lap_complete.ogg**
- Duration: 1.0-2.0 seconds
- Type: Notification sound
- Usage: Completing a lap

**race_start.ogg**
- Duration: 1.0-2.0 seconds
- Type: Signal beep
- Usage: Race countdown/start

**race_finish.ogg**
- Duration: 1.0-3.0 seconds
- Type: Victory chime
- Usage: Race completion

**horn.ogg**
- Duration: 0.5-1.0 seconds
- Type: Vehicle horn
- Usage: Player horn activation

**damage.ogg**
- Duration: 0.5-1.0 seconds
- Type: Crash/scrape sound
- Usage: Vehicle collision/damage

---

## 🎧 Audio Tools

### Recommended Tools to Create/Convert Audio

**Free Options:**
- **Audacity** - Freely available audio editor
  - Website: https://www.audacityteam.org/
  - Can export to OGG format
  - Can adjust pitch and length

**Online Tools:**
- **CloudConvert** - Audio format conversion
  - Website: https://cloudconvert.com/
  - Supports MP3 → OGG conversion

- **Online Audio Converter**
  - Website: https://online-audio-converter.com/
  - Simple web-based conversion

**Professional Tools:**
- **Adobe Audition** - Professional audio editing
- **Reaper** - DAW for audio creation
- **FL Studio** - Music production

### How to Create Engine Sounds

**Using Audacity:**
1. Generate tone at target frequency (Analyze → Tone Generator)
2. Adjust pitch envelope (low → high for acceleration effect)
3. Add slight distortion (Effect → Distortion)
4. Fade in/out edges
5. Export as OGG Vorbis

**Using FFmpeg (Command Line):**
```bash
# Convert MP3 to OGG
ffmpeg -i input.mp3 -c:a libvorbis -q:a 6 output.ogg

# Adjust volume
ffmpeg -i input.mp3 -af "volume=1.5" output.ogg

# Speed up (increase pitch effect)
ffmpeg -i input.mp3 -filter:a "atempo=1.2" output.ogg
```

---

## 📝 Audio Configuration

### Default Configuration (GameConfig)

```cpp
// In GameManager::GameConfig
struct GameConfig {
    // ... existing config ...
    
    // Audio paths (relative to executable)
    std::string music_path = "assets/audio/music/";
    std::string engine_path = "assets/audio/engine/";
    std::string sfx_path = "assets/audio/sfx/";
    
    // Volume levels (0.0 - 1.0)
    float master_volume = 0.8f;     // Overall volume
    float music_volume = 0.6f;      // Background music
    float engine_volume = 0.7f;     // Engine sounds
    float sfx_volume = 0.8f;        // Sound effects
    
    // Audio settings
    bool audio_enabled = true;
    int audio_channels = 32;        // Max simultaneous sounds
    int sample_rate = 44100;        // Hz
};
```

---

## 🔊 Audio System Architecture

### How Audio is Loaded

```cpp
// AudioSystem.cpp initialization
void AudioSystem::Initialize(const GameConfig& config) {
    // 1. Initialize OpenAL context
    InitializeOpenAL();
    
    // 2. Load music files
    LoadMusic(config.music_path + "menu.ogg");
    LoadMusic(config.music_path + "racing.ogg");
    LoadMusic(config.music_path + "victory.ogg");
    
    // 3. Load engine sounds
    LoadEngineSound("idle", config.engine_path + "idle.ogg");
    LoadEngineSound("low", config.engine_path + "low.ogg");
    LoadEngineSound("mid", config.engine_path + "mid.ogg");
    LoadEngineSound("high", config.engine_path + "high.ogg");
    LoadEngineSound("max", config.engine_path + "max.ogg");
    
    // 4. Load SFX files
    LoadSFX("shift_up", config.sfx_path + "shift_up.ogg");
    LoadSFX("shift_down", config.sfx_path + "shift_down.ogg");
    LoadSFX("collision", config.sfx_path + "collision.ogg");
    LoadSFX("lap_complete", config.sfx_path + "lap_complete.ogg");
    LoadSFX("race_start", config.sfx_path + "race_start.ogg");
    LoadSFX("race_finish", config.sfx_path + "race_finish.ogg");
    LoadSFX("horn", config.sfx_path + "horn.ogg");
    LoadSFX("damage", config.sfx_path + "damage.ogg");
}
```

### How Audio is Played

```cpp
// During race
void AudioSystem::PlayEngineSound(float speed_km_h) {
    // Select appropriate engine sound based on speed
    if (speed_km_h < 10) {
        PlayLooped("idle");
    } else if (speed_km_h < 50) {
        PlayLooped("low");
    } else if (speed_km_h < 150) {
        PlayLooped("mid");
    } else if (speed_km_h < 250) {
        PlayLooped("high");
    } else {
        PlayLooped("max");
    }
}

// SFX events
void AudioSystem::PlaySFX(const std::string& name) {
    // Play one-time sound effect
    PlayOnce(name);
}

// Music management
void AudioSystem::PlayMusic(const std::string& name) {
    StopMusic();
    PlayLooped(name);
}
```

---

## ⚙️ Audio Settings Dialog

The settings dialog will include:

```
╔════════════════════════════════════════════════════════════╗
║              AUDIO SETTINGS                                ║
╠════════════════════════════════════════════════════════════╣
║                                                            ║
║  Master Volume:      [████████░░] 80%                      ║
║  Music Volume:       [██████░░░░] 60%                      ║
║  Engine Volume:      [███████░░░] 70%                      ║
║  SFX Volume:         [████████░░] 80%                      ║
║                                                            ║
║  ☑ Audio Enabled                                          ║
║  ☑ Engine Sounds                                          ║
║  ☑ Music                                                  ║
║  ☑ Sound Effects                                          ║
║                                                            ║
║  Audio Device:       [Default Device          ▼]          ║
║  Sample Rate:        [44100 Hz                ▼]          ║
║  Channels:           [32                      ▼]          ║
║                                                            ║
║  Test Sounds:                                             ║
║  [▶ Engine Idle]  [▶ Shift Up]  [▶ Race Start]           ║
║                                                            ║
║  [APPLY]  [RESET TO DEFAULTS]  [CLOSE]                   ║
║                                                            ║
╚════════════════════════════════════════════════════════════╝
```

---

## 📥 Where to Get Free Audio

### Free Music Sites
- **Freepd.com** - Royalty-free music
- **Incompetech.com** - Royalty-free music
- **Bensound.com** - Free background music
- **Zapsplat.com** - Free sound effects and music
- **Opengameart.org** - Game-specific assets

### Free Sound Effects
- **Freesound.org** - Community sound effects
- **Pixabay.com** - Royalty-free sound effects
- **Zapsplat.com** - Sound effects library
- **BBC Sound Effects Library** - Extensive library

### How to Use
1. Download audio file (preferably MP3 or WAV)
2. Convert to OGG using tool listed above
3. Place in appropriate asset folder
4. Game will load automatically

---

## 🎯 Testing Audio

### Manual Testing

1. **Load Game**
   ```bash
   ./TopSpeed
   ```

2. **Check Console Output**
   ```
   [Audio] Initializing AudioSystem...
   [Audio] Loading music files...
   [Audio] Loaded: menu.ogg
   [Audio] Loaded: racing.ogg
   [Audio] Loading engine sounds...
   [Audio] Loaded: idle.ogg
   [Audio] Loaded: low.ogg
   ...
   [Audio] Ready!
   ```

3. **Start Race**
   - Should hear engine sounds
   - Volume should match configuration
   - Pitch should change with speed

4. **Listen for Events**
   - Gear shifts (if manual mode)
   - Collisions
   - Lap completion
   - Race finish

### Audio Debug Info

In settings dialog or console:
```
Audio System Status:
- Device: Default Device
- Active Sources: 3/32
- Master Volume: 80%
- Playing: racing.ogg (looped)
- Engine Sound: mid.ogg (looped)
- SFX Queue: empty
```

---

## 📋 Troubleshooting

### No Sound

1. **Check audio files exist**
   ```bash
   ls -la assets/audio/music/
   ls -la assets/audio/engine/
   ls -la assets/audio/sfx/
   ```

2. **Check file permissions**
   ```bash
   chmod 644 assets/audio/*/*.ogg
   ```

3. **Check audio settings**
   - Open Settings Dialog
   - Verify volumes not at 0%
   - Verify "Audio Enabled" is checked
   - Click "Test Sounds" button

4. **Check OpenAL installation**
   ```bash
   # Linux
   sudo apt install libopenal-dev libogg-dev libvorbis-dev
   
   # macOS
   brew install openal-soft libogg libvorbis
   
   # Windows
   # Download from https://www.openal.org/
   ```

5. **Check system volume**
   - Verify OS volume is not muted
   - Verify application is not muted in mixer

### Audio Crackles/Pops

1. Check file format (must be OGG Vorbis)
2. Try different audio device in settings
3. Increase buffer size in config
4. Verify file sample rate (should be 44100 Hz)

### Engine Sound Doesn't Change Pitch

1. Verify engine sound files loaded
2. Check speed is actually changing
3. Check volume isn't too low
4. Review engine sound selection logic

---

## 📝 Configuration File

If you want to manually edit config (before settings dialog):

```json
{
  "audio": {
    "enabled": true,
    "master_volume": 0.8,
    "music_volume": 0.6,
    "engine_volume": 0.7,
    "sfx_volume": 0.8,
    "paths": {
      "music": "assets/audio/music/",
      "engine": "assets/audio/engine/",
      "sfx": "assets/audio/sfx/"
    },
    "device": "default",
    "sample_rate": 44100,
    "channels": 32
  }
}
```

---

## ✅ Checklist for Audio Setup

- [ ] Create `assets/audio/` directory structure
- [ ] Place music files in `assets/audio/music/`
- [ ] Place engine sounds in `assets/audio/engine/`
- [ ] Place SFX in `assets/audio/sfx/`
- [ ] All files are OGG format
- [ ] All files are 44100 Hz sample rate
- [ ] Rebuild game with new audio system
- [ ] Test with Settings Dialog
- [ ] Adjust volumes as needed
- [ ] Test audio on target hardware

---

## 🎵 Summary

**Audio Setup is Easy:**

1. Create folder structure: `assets/audio/{music,engine,sfx}/`
2. Place OGG files in appropriate folders
3. Open game and use Settings Dialog
4. Adjust volumes to your liking
5. Click "Test Sounds" to verify
6. Enjoy the audio!

**No manual config file editing needed!** 🎧
