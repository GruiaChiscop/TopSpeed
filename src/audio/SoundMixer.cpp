#include "SoundMixer.h"
#include <sstream>
#include <iostream>

namespace TopSpeed {

// ============================================================================
// SoundMixer Implementation
// ============================================================================

SoundMixer::SoundMixer(const std::string& name)
    : name_(name), volume_(1.0f), target_volume_(1.0f), fade_duration_(0.0f),
      fade_elapsed_(0.0f), muted_(false), enabled_(true), solo_(false) {}

SoundMixer::~SoundMixer() = default;

void SoundMixer::AddSound(uint32_t sound_id) {
    // Check if already in mixer
    for (uint32_t id : sounds_) {
        if (id == sound_id) {
            return;  // Already added
        }
    }
    sounds_.push_back(sound_id);
}

void SoundMixer::RemoveSound(uint32_t sound_id) {
    auto it = std::find(sounds_.begin(), sounds_.end(), sound_id);
    if (it != sounds_.end()) {
        sounds_.erase(it);
    }
}

bool SoundMixer::HasSound(uint32_t sound_id) const {
    return std::find(sounds_.begin(), sounds_.end(), sound_id) != sounds_.end();
}

void SoundMixer::ClearSounds() {
    sounds_.clear();
}

void SoundMixer::AddEffect(std::unique_ptr<AudioEffect> effect) {
    if (effect) {
        effects_.push_back(std::move(effect));
    }
}

void SoundMixer::RemoveEffect(size_t index) {
    if (index < effects_.size()) {
        effects_.erase(effects_.begin() + index);
    }
}

AudioEffect* SoundMixer::GetEffect(size_t index) {
    if (index < effects_.size()) {
        return effects_[index].get();
    }
    return nullptr;
}

void SoundMixer::ClearEffects() {
    effects_.clear();
}

void SoundMixer::SetVolume(float volume) {
    volume_ = std::clamp(volume, 0.0f, 1.0f);
    target_volume_ = volume_;
    fade_duration_ = 0.0f;  // Cancel any active fade
}

void SoundMixer::FadeVolume(float target, float duration_ms) {
    target_volume_ = std::clamp(target, 0.0f, 1.0f);
    fade_duration_ = duration_ms / 1000.0f;  // Convert to seconds
    fade_elapsed_ = 0.0f;
}

float SoundMixer::ProcessSample(float sample) {
    if (!enabled_ || muted_) {
        return 0.0f;
    }
    
    // Apply effect chain in order
    float processed = sample;
    for (auto& effect : effects_) {
        if (effect && effect->IsEnabled()) {
            processed = effect->Process(processed);
        }
    }
    
    // Apply mixer volume
    return processed * volume_;
}

void SoundMixer::Update(float delta_time) {
    // Update volume fade
    if (fade_duration_ > 0.0f) {
        fade_elapsed_ += delta_time;
        
        if (fade_elapsed_ >= fade_duration_) {
            volume_ = target_volume_;
            fade_duration_ = 0.0f;  // Fade complete
        } else {
            // Linear interpolation
            float t = fade_elapsed_ / fade_duration_;
            float start_volume = volume_;  // Store starting volume
            
            // Use target as the point we're fading to
            if (fade_elapsed_ == delta_time) {
                // First frame of fade - store current as start
                start_volume = volume_;
            }
            
            // Simple linear fade
            volume_ += (target_volume_ - volume_) * (delta_time / fade_duration_);
            volume_ = std::clamp(volume_, 0.0f, 1.0f);
        }
    }
}

void SoundMixer::Reset() {
    for (auto& effect : effects_) {
        if (effect) {
            effect->Reset();
        }
    }
}

std::string SoundMixer::GetDebugInfo() const {
    std::ostringstream oss;
    oss << "Mixer: " << name_ << "\n";
    oss << "  Sounds: " << sounds_.size() << "\n";
    oss << "  Volume: " << (volume_ * 100.0f) << "%";
    if (fade_duration_ > 0.0f) {
        oss << " (fading to " << (target_volume_ * 100.0f) << "%)";
    }
    oss << "\n";
    oss << "  Effects: " << effects_.size() << "\n";
    oss << "  Enabled: " << (enabled_ ? "yes" : "no") << "\n";
    oss << "  Muted: " << (muted_ ? "yes" : "no") << "\n";
    oss << "  Solo: " << (solo_ ? "yes" : "no");
    return oss.str();
}

// ============================================================================
// MixerManager Implementation
// ============================================================================

SoundMixer* MixerManager::CreateMixer(const std::string& name) {
    // Check if mixer already exists
    auto it = mixers_.find(name);
    if (it != mixers_.end()) {
        return it->second.get();
    }
    
    // Create new mixer
    auto mixer = std::make_unique<SoundMixer>(name);
    SoundMixer* ptr = mixer.get();
    mixers_[name] = std::move(mixer);
    
    std::cout << "Created mixer: " << name << std::endl;
    return ptr;
}

SoundMixer* MixerManager::GetMixer(const std::string& name) {
    auto it = mixers_.find(name);
    if (it != mixers_.end()) {
        return it->second.get();
    }
    return nullptr;
}

void MixerManager::RemoveMixer(const std::string& name) {
    mixers_.erase(name);
}

void MixerManager::ClearMixers() {
    mixers_.clear();
}

void MixerManager::SetMasterVolume(float volume) {
    master_volume_ = std::clamp(volume, 0.0f, 1.0f);
}

void MixerManager::Update(float delta_time) {
    for (auto& [name, mixer] : mixers_) {
        if (mixer) {
            mixer->Update(delta_time);
        }
    }
}

float MixerManager::ProcessSample(const std::string& mixer_name, float sample) {
    auto mixer = GetMixer(mixer_name);
    if (mixer) {
        float processed = mixer->ProcessSample(sample);
        return processed * master_volume_;
    }
    return sample * master_volume_;
}

std::string MixerManager::GetDebugInfo() const {
    std::ostringstream oss;
    oss << "=== Mixer Manager ===\n";
    oss << "Master Volume: " << (master_volume_ * 100.0f) << "%\n";
    oss << "Total Mixers: " << mixers_.size() << "\n\n";
    
    for (const auto& [name, mixer] : mixers_) {
        oss << mixer->GetDebugInfo() << "\n\n";
    }
    
    return oss.str();
}

} // namespace TopSpeed
