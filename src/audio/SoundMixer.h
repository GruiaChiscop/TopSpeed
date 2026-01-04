#pragma once

#include "Effects.h"
#include <vector>
#include <map>
#include <memory>
#include <string>

namespace TopSpeed {

/**
 * Sound mixer - groups sounds together with shared effects chain
 * 
 * Pattern inspired by NVGT's sound mixer system:
 * - Group multiple sounds (e.g., all SFX, all music)
 * - Apply effects chain to entire group
 * - Control group volume independently
 * - Real-time effect parameter tweaking
 * 
 * Usage:
 *   SoundMixer sfx_mixer;
 *   sfx_mixer.AddSound(crash_id);
 *   sfx_mixer.AddSound(beep_id);
 *   sfx_mixer.AddEffect(std::make_unique<DistortionEffect>(2.0f));
 *   sfx_mixer.SetVolume(0.8f);
 */
class SoundMixer {
public:
    /**
     * Create mixer for a specific context
     * @param name Mixer name ("music", "sfx", "ui", "dialogue")
     */
    explicit SoundMixer(const std::string& name = "mixer");
    ~SoundMixer();
    
    // Sound management
    /**
     * Add sound to this mixer
     * @param sound_id Audio engine sound ID
     */
    void AddSound(uint32_t sound_id);
    
    /**
     * Remove sound from mixer
     */
    void RemoveSound(uint32_t sound_id);
    
    /**
     * Check if sound is in this mixer
     */
    bool HasSound(uint32_t sound_id) const;
    
    /**
     * Get number of sounds in mixer
     */
    size_t GetSoundCount() const { return sounds_.size(); }
    
    /**
     * Remove all sounds from mixer
     */
    void ClearSounds();
    
    // Effect chain management
    /**
     * Add effect to end of effect chain
     * Effects are applied in order (FIFO)
     * @param effect Unique pointer to effect (mixer takes ownership)
     */
    void AddEffect(std::unique_ptr<AudioEffect> effect);
    
    /**
     * Remove effect at index from chain
     */
    void RemoveEffect(size_t index);
    
    /**
     * Get number of effects in chain
     */
    size_t GetEffectCount() const { return effects_.size(); }
    
    /**
     * Get effect at index (nullptr if out of bounds)
     */
    AudioEffect* GetEffect(size_t index);
    
    /**
     * Clear all effects from chain
     */
    void ClearEffects();
    
    // Volume control
    /**
     * Set mixer volume (independent of source volumes)
     * @param volume Volume level (0.0 = silent, 1.0 = full)
     */
    void SetVolume(float volume);
    
    /**
     * Get current mixer volume
     */
    float GetVolume() const { return volume_; }
    
    /**
     * Fade volume over time
     * @param target Target volume
     * @param duration_ms Fade duration in milliseconds
     */
    void FadeVolume(float target, float duration_ms);
    
    // Mixer state
    /**
     * Mute/unmute mixer
     */
    void SetMuted(bool muted) { muted_ = muted; }
    bool IsMuted() const { return muted_; }
    
    /**
     * Enable/disable mixer
     */
    void SetEnabled(bool enabled) { enabled_ = enabled; }
    bool IsEnabled() const { return enabled_; }
    
    /**
     * Solo this mixer (audio engine will only play this mixer)
     * @param solo True to solo, false to unsolo
     */
    void SetSolo(bool solo) { solo_ = solo; }
    bool IsSolo() const { return solo_; }
    
    // Processing
    /**
     * Process audio sample through effect chain
     * Called by mixer manager for each sample from this mixer
     * @param sample Input sample (-1.0 to 1.0)
     * @return Processed sample with effects applied
     */
    float ProcessSample(float sample);
    
    /**
     * Update mixer (for fade effects, etc.)
     * Called by mixer manager each frame
     * @param delta_time Frame time in seconds
     */
    void Update(float delta_time);
    
    /**
     * Reset all effects
     */
    void Reset();
    
    // Information
    /**
     * Get mixer name
     */
    const std::string& GetName() const { return name_; }
    
    /**
     * Get debug info string
     */
    std::string GetDebugInfo() const;

private:
    std::string name_;
    std::vector<uint32_t> sounds_;              // Sound IDs in this mixer
    std::vector<std::unique_ptr<AudioEffect>> effects_; // Effect chain
    
    float volume_ = 1.0f;
    float target_volume_ = 1.0f;
    float fade_duration_ = 0.0f;
    float fade_elapsed_ = 0.0f;
    bool muted_ = false;
    bool enabled_ = true;
    bool solo_ = false;
    
    // Fade helper
    void UpdateFade(float delta_time);
};

/**
 * Mixer manager - coordinates multiple mixers
 * 
 * Typical usage:
 *   MixerManager manager;
 *   manager.CreateMixer("music");
 *   manager.CreateMixer("sfx");
 *   manager.CreateMixer("ui");
 *   manager.GetMixer("sfx")->AddSound(crash_id);
 */
class MixerManager {
public:
    /**
     * Create a new mixer
     * @param name Mixer name (must be unique)
     * @return Pointer to created mixer
     */
    SoundMixer* CreateMixer(const std::string& name);
    
    /**
     * Get mixer by name
     * @return Pointer to mixer or nullptr if not found
     */
    SoundMixer* GetMixer(const std::string& name);
    
    /**
     * Remove mixer by name
     */
    void RemoveMixer(const std::string& name);
    
    /**
     * Get number of active mixers
     */
    size_t GetMixerCount() const { return mixers_.size(); }
    
    /**
     * Remove all mixers
     */
    void ClearMixers();
    
    /**
     * Get master volume (applies to all mixers)
     */
    void SetMasterVolume(float volume);
    float GetMasterVolume() const { return master_volume_; }
    
    /**
     * Update all mixers
     */
    void Update(float delta_time);
    
    /**
     * Process sample through specified mixer
     */
    float ProcessSample(const std::string& mixer_name, float sample);
    
    /**
     * Get mixer info for debugging
     */
    std::string GetDebugInfo() const;
    
    /**
     * Standard mixer names (for convenience)
     */
    static constexpr const char* MIXER_MUSIC = "music";
    static constexpr const char* MIXER_SFX = "sfx";
    static constexpr const char* MIXER_UI = "ui";
    static constexpr const char* MIXER_DIALOGUE = "dialogue";
    static constexpr const char* MIXER_MASTER = "master";

private:
    std::map<std::string, std::unique_ptr<SoundMixer>> mixers_;
    float master_volume_ = 1.0f;
};

} // namespace TopSpeed
