#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

/**
 * @class AudioSystem
 * @brief Central audio management system for the game
 * 
 * Manages all sound effects, music, and audio playback.
 * Supports pooling, volume control, and event-based audio triggers.
 */
class AudioSystem {
public:
    enum class AudioType {
        SoundEffect,    // One-shot sound
        Music,          // Background music
        Ambient,        // Looping ambient sound
        Voice           // Speech/callouts
    };

    enum class SoundCategory {
        Engine,         // Vehicle engine sounds
        Road,           // Road/surface sounds
        Crash,          // Collision sounds
        UI,             // Menu/UI sounds
        Ambient,        // Environmental sounds
        Voice,          // Speech and callouts
        Music           // Background music
    };

    struct AudioSource {
        std::string id;
        std::string filepath;
        AudioType type;
        SoundCategory category;
        float volume = 1.0f;
        float pitch = 1.0f;
        bool looping = false;
        bool is_playing = false;
    };

    AudioSystem();
    ~AudioSystem();

    // Initialization
    bool Initialize();
    void Shutdown();

    // Audio loading
    bool LoadAudio(const std::string& id, const std::string& filepath,
                   AudioType type, SoundCategory category);
    bool UnloadAudio(const std::string& id);
    void UnloadAll();

    // Audio playback
    void PlaySound(const std::string& id, float volume = 1.0f, float pitch = 1.0f);
    void PlayMusic(const std::string& id, float volume = 1.0f, bool fade_in = false);
    void StopMusic(bool fade_out = false);
    void StopSound(const std::string& id);
    void StopAllSounds();

    // Volume control
    void SetMasterVolume(float volume);
    float GetMasterVolume() const { return master_volume_; }
    void SetCategoryVolume(SoundCategory category, float volume);
    float GetCategoryVolume(SoundCategory category) const;

    // Audio source management
    void PauseSound(const std::string& id);
    void ResumeSound(const std::string& id);
    void PauseAll();
    void ResumeAll();

    // Querying
    bool IsAudioLoaded(const std::string& id) const;
    bool IsAudioPlaying(const std::string& id) const;
    std::shared_ptr<AudioSource> GetAudioSource(const std::string& id);

    // Update (call from main game loop)
    void Update(float deltaTime);

    // Debug info
    std::string GetDebugInfo() const;

private:
    struct PlayingSound {
        std::shared_ptr<AudioSource> source;
        float elapsed_time = 0.0f;
        float duration = 0.0f;
    };

    std::unordered_map<std::string, std::shared_ptr<AudioSource>> loaded_audio_;
    std::vector<PlayingSound> playing_sounds_;
    std::string current_music_id_;

    float master_volume_ = 1.0f;
    std::unordered_map<int, float> category_volumes_;

    bool initialized_ = false;
    bool audio_paused_ = false;

    // Helper methods
    void InitializeCategoryVolumes();
    void UpdatePlayingSounds(float deltaTime);
    void CleanupFinishedSounds();
};
