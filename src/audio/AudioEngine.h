#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

namespace TopSpeed {

/**
 * Audio source representation
 */
struct AudioSource {
    uint32_t id;
    std::string filename;
    float volume;
    bool looping;
    bool playing;
};

/**
 * AudioEngine provides modern audio playback using miniaudio
 * Replaces legacy DirectX audio functionality with Ogg Vorbis support
 */
class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    /**
     * Initialize audio subsystem
     * @return true if successful
     */
    bool Initialize();

    /**
     * Load an audio file (Ogg Vorbis format)
     * @param filename Path to audio file
     * @return Audio source ID, or 0 on failure
     */
    uint32_t LoadAudio(const std::string& filename);

    /**
     * Play audio source
     * @param sourceId ID returned from LoadAudio
     * @param looping Whether to loop the audio
     */
    void Play(uint32_t sourceId, bool looping = false);

    /**
     * Stop audio playback
     * @param sourceId ID of audio to stop
     */
    void Stop(uint32_t sourceId);

    /**
     * Pause audio playback
     * @param sourceId ID of audio to pause
     */
    void Pause(uint32_t sourceId);

    /**
     * Resume paused audio
     * @param sourceId ID of audio to resume
     */
    void Resume(uint32_t sourceId);

    /**
     * Set volume for audio source (0.0 = silent, 1.0 = normal)
     * @param sourceId ID of audio source
     * @param volume Volume level
     */
    void SetVolume(uint32_t sourceId, float volume);

    /**
     * Check if audio is currently playing
     * @param sourceId ID of audio source
     * @return true if currently playing
     */
    bool IsPlaying(uint32_t sourceId) const;

    /**
     * Unload audio resource
     * @param sourceId ID of audio to unload
     */
    void Unload(uint32_t sourceId);

    /**
     * Unload all audio resources
     */
    void UnloadAll();

    /**
     * Set master volume (0.0 = silent, 1.0 = normal)
     * @param volume Master volume level
     */
    void SetMasterVolume(float volume);

    /**
     * Get master volume
     * @return Current master volume level
     */
    float GetMasterVolume() const { return master_volume_; }

    /**
     * Update audio engine (call once per frame)
     */
    void Update();

    /**
     * Shutdown audio engine
     */
    void Shutdown();

private:
    // NOTE: Placeholder implementation using miniaudio
    // Actual implementation requires miniaudio integration
    
    std::unordered_map<uint32_t, AudioSource> sources_;
    uint32_t next_source_id_;
    float master_volume_;
    bool initialized_;

    uint32_t AllocateSourceId();
};

} // namespace TopSpeed
