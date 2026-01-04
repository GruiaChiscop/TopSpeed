#include "AudioSystem.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <iostream>

AudioSystem::AudioSystem() {
    InitializeCategoryVolumes();
}

AudioSystem::~AudioSystem() {
    if (initialized_) {
        Shutdown();
    }
}

bool AudioSystem::Initialize() {
    if (initialized_) {
        return true;
    }

    // Initialize category volumes
    InitializeCategoryVolumes();

    initialized_ = true;
    return true;
}

void AudioSystem::Shutdown() {
    if (!initialized_) {
        return;
    }

    StopAllSounds();
    UnloadAll();
    initialized_ = false;
}

void AudioSystem::InitializeCategoryVolumes() {
    category_volumes_[static_cast<int>(SoundCategory::Engine)] = 1.0f;
    category_volumes_[static_cast<int>(SoundCategory::Road)] = 0.8f;
    category_volumes_[static_cast<int>(SoundCategory::Crash)] = 1.0f;
    category_volumes_[static_cast<int>(SoundCategory::UI)] = 0.7f;
    category_volumes_[static_cast<int>(SoundCategory::Ambient)] = 0.6f;
    category_volumes_[static_cast<int>(SoundCategory::Voice)] = 1.0f;
    category_volumes_[static_cast<int>(SoundCategory::Music)] = 0.6f;
}

bool AudioSystem::LoadAudio(const std::string& id, const std::string& filepath,
                            AudioType type, SoundCategory category) {
    // Check if already loaded
    if (loaded_audio_.find(id) != loaded_audio_.end()) {
        return true;  // Already loaded
    }

    auto audio_source = std::make_shared<AudioSource>();
    audio_source->id = id;
    audio_source->filepath = filepath;
    audio_source->type = type;
    audio_source->category = category;
    audio_source->volume = 1.0f;
    audio_source->pitch = 1.0f;
    audio_source->looping = (type == AudioType::Music || type == AudioType::Ambient);
    audio_source->is_playing = false;

    // Store the audio source
    loaded_audio_[id] = audio_source;

    return true;
}

bool AudioSystem::UnloadAudio(const std::string& id) {
    StopSound(id);
    auto it = loaded_audio_.find(id);
    if (it != loaded_audio_.end()) {
        loaded_audio_.erase(it);
        return true;
    }
    return false;
}

void AudioSystem::UnloadAll() {
    StopAllSounds();
    loaded_audio_.clear();
}

void AudioSystem::PlaySound(const std::string& id, float volume, float pitch) {
    auto it = loaded_audio_.find(id);
    if (it == loaded_audio_.end()) {
        return;  // Audio not loaded
    }

    auto source = it->second;
    source->volume = volume;
    source->pitch = pitch;
    source->is_playing = true;

    // Add to playing sounds
    PlayingSound playing;
    playing.source = source;
    playing.elapsed_time = 0.0f;
    playing.duration = 2.0f;  // Default duration (would be file duration in real impl)

    playing_sounds_.push_back(playing);
}

void AudioSystem::PlayMusic(const std::string& id, float volume, bool fade_in) {
    auto it = loaded_audio_.find(id);
    if (it == loaded_audio_.end()) {
        return;
    }

    // Stop current music
    if (!current_music_id_.empty()) {
        StopMusic(false);
    }

    auto source = it->second;
    source->volume = volume;
    source->is_playing = true;
    source->looping = true;

    // Add to playing sounds
    PlayingSound playing;
    playing.source = source;
    playing.elapsed_time = 0.0f;
    playing.duration = -1.0f;  // Infinite duration for music

    playing_sounds_.push_back(playing);
    current_music_id_ = id;
}

void AudioSystem::StopMusic(bool fade_out) {
    if (current_music_id_.empty()) {
        return;
    }

    StopSound(current_music_id_);
    current_music_id_.clear();
}

void AudioSystem::StopSound(const std::string& id) {
    auto it = std::find_if(playing_sounds_.begin(), playing_sounds_.end(),
                           [&id](const PlayingSound& ps) {
                               return ps.source && ps.source->id == id;
                           });

    if (it != playing_sounds_.end()) {
        it->source->is_playing = false;
        playing_sounds_.erase(it);
    }
}

void AudioSystem::StopAllSounds() {
    for (auto& sound : playing_sounds_) {
        if (sound.source) {
            sound.source->is_playing = false;
        }
    }
    playing_sounds_.clear();
    current_music_id_.clear();
}

void AudioSystem::SetMasterVolume(float volume) {
    master_volume_ = std::max(0.0f, std::min(1.0f, volume));
}

void AudioSystem::SetCategoryVolume(SoundCategory category, float volume) {
    float clamped = std::max(0.0f, std::min(1.0f, volume));
    category_volumes_[static_cast<int>(category)] = clamped;
}

float AudioSystem::GetCategoryVolume(SoundCategory category) const {
    auto it = category_volumes_.find(static_cast<int>(category));
    if (it != category_volumes_.end()) {
        return it->second;
    }
    return 1.0f;
}

void AudioSystem::PauseSound(const std::string& id) {
    auto it = std::find_if(playing_sounds_.begin(), playing_sounds_.end(),
                           [&id](const PlayingSound& ps) {
                               return ps.source && ps.source->id == id;
                           });

    if (it != playing_sounds_.end() && it->source) {
        it->source->is_playing = false;
    }
}

void AudioSystem::ResumeSound(const std::string& id) {
    auto it = std::find_if(playing_sounds_.begin(), playing_sounds_.end(),
                           [&id](const PlayingSound& ps) {
                               return ps.source && ps.source->id == id;
                           });

    if (it != playing_sounds_.end() && it->source) {
        it->source->is_playing = true;
    }
}

void AudioSystem::PauseAll() {
    audio_paused_ = true;
    for (auto& sound : playing_sounds_) {
        if (sound.source) {
            sound.source->is_playing = false;
        }
    }
}

void AudioSystem::ResumeAll() {
    audio_paused_ = false;
    for (auto& sound : playing_sounds_) {
        if (sound.source) {
            sound.source->is_playing = true;
        }
    }
}

bool AudioSystem::IsAudioLoaded(const std::string& id) const {
    return loaded_audio_.find(id) != loaded_audio_.end();
}

bool AudioSystem::IsAudioPlaying(const std::string& id) const {
    return std::any_of(playing_sounds_.begin(), playing_sounds_.end(),
                       [&id](const PlayingSound& ps) {
                           return ps.source && ps.source->id == id && ps.source->is_playing;
                       });
}

std::shared_ptr<AudioSystem::AudioSource> AudioSystem::GetAudioSource(const std::string& id) {
    auto it = loaded_audio_.find(id);
    if (it != loaded_audio_.end()) {
        return it->second;
    }
    return nullptr;
}

void AudioSystem::Update(float deltaTime) {
    if (!initialized_ || audio_paused_) {
        return;
    }

    UpdatePlayingSounds(deltaTime);
    CleanupFinishedSounds();
}

void AudioSystem::UpdatePlayingSounds(float deltaTime) {
    for (auto& sound : playing_sounds_) {
        if (sound.source && sound.source->is_playing) {
            sound.elapsed_time += deltaTime;
        }
    }
}

void AudioSystem::CleanupFinishedSounds() {
    auto it = playing_sounds_.begin();
    while (it != playing_sounds_.end()) {
        if (!it->source->looping && it->elapsed_time >= it->duration) {
            it->source->is_playing = false;
            it = playing_sounds_.erase(it);
        } else {
            ++it;
        }
    }
}

std::string AudioSystem::GetDebugInfo() const {
    std::stringstream ss;
    ss << "AudioSystem Debug Info\n";
    ss << "- Initialized: " << (initialized_ ? "Yes" : "No") << "\n";
    ss << "- Paused: " << (audio_paused_ ? "Yes" : "No") << "\n";
    ss << "- Master Volume: " << std::fixed << std::setprecision(2) << master_volume_ << "\n";
    ss << "- Loaded Audio: " << loaded_audio_.size() << "\n";
    ss << "- Playing Sounds: " << playing_sounds_.size() << "\n";
    ss << "- Current Music: " << (current_music_id_.empty() ? "None" : current_music_id_) << "\n";
    ss << "\nCategory Volumes:\n";
    ss << "  Engine: " << std::fixed << std::setprecision(2) << GetCategoryVolume(SoundCategory::Engine) << "\n";
    ss << "  Road: " << GetCategoryVolume(SoundCategory::Road) << "\n";
    ss << "  Crash: " << GetCategoryVolume(SoundCategory::Crash) << "\n";
    ss << "  UI: " << GetCategoryVolume(SoundCategory::UI) << "\n";
    ss << "  Ambient: " << GetCategoryVolume(SoundCategory::Ambient) << "\n";
    ss << "  Voice: " << GetCategoryVolume(SoundCategory::Voice) << "\n";
    ss << "  Music: " << GetCategoryVolume(SoundCategory::Music) << "\n";
    return ss.str();
}
