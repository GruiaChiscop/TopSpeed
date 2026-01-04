#include "RaceAudio.h"
#include "../game/RaceManager.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

RaceAudio::RaceAudio(RaceManager* race_manager, AudioSystem* audio_system)
    : race_manager_(race_manager), audio_system_(audio_system) {
}

RaceAudio::~RaceAudio() {
    Shutdown();
}

bool RaceAudio::Initialize() {
    if (!audio_system_) {
        return false;
    }

    PreloadRaceSounds();
    PreloadRaceMusic();

    return true;
}

void RaceAudio::Shutdown() {
    if (audio_system_) {
        StopRaceMusic();
        audio_system_->StopAllSounds();
    }
    callout_queue_.clear();
    callout_cooldowns_.clear();
}

void RaceAudio::PreloadRaceSounds() {
    if (!audio_system_) return;

    // Countdown sounds
    audio_system_->LoadAudio(countdown_3_sound_id_, "assets/audio/countdown/countdown_3.wav",
                            AudioSystem::AudioType::SoundEffect, AudioSystem::SoundCategory::Voice);
    audio_system_->LoadAudio(countdown_2_sound_id_, "assets/audio/countdown/countdown_2.wav",
                            AudioSystem::AudioType::SoundEffect, AudioSystem::SoundCategory::Voice);
    audio_system_->LoadAudio(countdown_1_sound_id_, "assets/audio/countdown/countdown_1.wav",
                            AudioSystem::AudioType::SoundEffect, AudioSystem::SoundCategory::Voice);
    audio_system_->LoadAudio(countdown_go_sound_id_, "assets/audio/countdown/countdown_go.wav",
                            AudioSystem::AudioType::SoundEffect, AudioSystem::SoundCategory::Voice);

    // Race event sounds
    audio_system_->LoadAudio(race_start_sound_id_, "assets/audio/race/race_start.wav",
                            AudioSystem::AudioType::SoundEffect, AudioSystem::SoundCategory::UI);
    audio_system_->LoadAudio(race_finish_sound_id_, "assets/audio/race/race_finish.wav",
                            AudioSystem::AudioType::SoundEffect, AudioSystem::SoundCategory::UI);
    audio_system_->LoadAudio(position_change_sound_id_, "assets/audio/race/position_change.wav",
                            AudioSystem::AudioType::SoundEffect, AudioSystem::SoundCategory::Voice);
    audio_system_->LoadAudio(warning_sound_id_, "assets/audio/race/warning.wav",
                            AudioSystem::AudioType::SoundEffect, AudioSystem::SoundCategory::Voice);
    audio_system_->LoadAudio(lap_complete_sound_id_, "assets/audio/race/lap_complete.wav",
                            AudioSystem::AudioType::SoundEffect, AudioSystem::SoundCategory::Voice);
}

void RaceAudio::PreloadRaceMusic() {
    if (!audio_system_) return;

    // Race music
    audio_system_->LoadAudio(race_music_id_, "assets/audio/music/race_main.wav",
                            AudioSystem::AudioType::Music, AudioSystem::SoundCategory::Music);
    audio_system_->LoadAudio(menu_music_id_, "assets/audio/music/menu.wav",
                            AudioSystem::AudioType::Music, AudioSystem::SoundCategory::Music);
    audio_system_->LoadAudio(ambient_music_id_, "assets/audio/music/ambient.wav",
                            AudioSystem::AudioType::Music, AudioSystem::SoundCategory::Music);
}

void RaceAudio::SetRacePhase(RacePhase phase) {
    current_phase_ = phase;
    phase_duration_ = 0.0f;

    switch (phase) {
        case RacePhase::PreRace:
            SetAmbientMusic(menu_music_id_);
            break;
        case RacePhase::Countdown:
            break;  // Handled by countdown sounds
        case RacePhase::RaceStart:
            PlayRaceStartSound();
            SetRaceMusic(race_music_id_, true);
            break;
        case RacePhase::Racing:
            // Music already playing
            break;
        case RacePhase::Finished:
            PlayRaceFinishSound();
            if (audio_system_) {
                audio_system_->StopMusic(true);
            }
            break;
    }
}

void RaceAudio::PlayCountdownBeep(int remaining_count) {
    if (!audio_system_) return;

    std::string sound_id;
    switch (remaining_count) {
        case 3:
            sound_id = countdown_3_sound_id_;
            break;
        case 2:
            sound_id = countdown_2_sound_id_;
            break;
        case 1:
            sound_id = countdown_1_sound_id_;
            break;
        case 0:  // Go!
            sound_id = countdown_go_sound_id_;
            break;
        default:
            return;
    }

    audio_system_->PlaySound(sound_id, callout_volume_, 1.0f);
}

void RaceAudio::PlayRaceStartSound() {
    if (!audio_system_) return;

    audio_system_->PlaySound(race_start_sound_id_, effect_volume_, 1.0f);
}

void RaceAudio::PlayRaceFinishSound() {
    if (!audio_system_) return;

    audio_system_->PlaySound(race_finish_sound_id_, effect_volume_, 1.0f);
}

void RaceAudio::PlayPositionChangeSound() {
    if (!audio_system_) return;

    audio_system_->PlaySound(position_change_sound_id_, effect_volume_, 1.0f);
}

void RaceAudio::PlayWarningSound() {
    if (!audio_system_) return;

    audio_system_->PlaySound(warning_sound_id_, effect_volume_, 1.0f);
}

void RaceAudio::QueueCallout(const Callout& callout) {
    if (CanPlayCalloutType(callout.type)) {
        callout_queue_.push_back(callout);
        SetCalloutCooldown(callout.type, callout.cooldown);
    }
}

void RaceAudio::QueuePositionCallout(int position, const std::string& opponent_name) {
    Callout callout;
    callout.type = CalloutType::PositionChange;
    std::stringstream ss;
    ss << "You are in position " << position;
    if (!opponent_name.empty()) {
        ss << ", behind " << opponent_name;
    }
    callout.text = ss.str();
    callout.duration = 3.0f;
    callout.cooldown = 2.0f;

    QueueCallout(callout);
}

void RaceAudio::QueueRoadCallout(const std::string& road_description) {
    Callout callout;
    callout.type = CalloutType::RoadDescription;
    callout.text = road_description;
    callout.duration = 2.0f;
    callout.cooldown = 1.0f;

    QueueCallout(callout);
}

void RaceAudio::QueueObstacleCallout(const std::string& obstacle_type) {
    Callout callout;
    callout.type = CalloutType::ObstacleWarning;
    callout.text = "Watch out for " + obstacle_type;
    callout.duration = 2.0f;
    callout.cooldown = 3.0f;

    QueueCallout(callout);
}

void RaceAudio::QueueLapCallout(int lap_number, float lap_time) {
    Callout callout;
    callout.type = CalloutType::LapComplete;
    std::stringstream ss;
    ss << "Lap " << lap_number << " complete";
    if (lap_time > 0.0f) {
        ss << ", time " << std::fixed << std::setprecision(2) << lap_time;
    }
    callout.text = ss.str();
    callout.duration = 2.5f;
    callout.cooldown = 2.0f;

    QueueCallout(callout);
}

void RaceAudio::SetRaceMusic(const std::string& music_id, bool fade_in) {
    if (!audio_system_) return;

    audio_system_->PlayMusic(music_id, music_volume_, fade_in);
    race_music_playing_ = true;
}

void RaceAudio::StopRaceMusic(bool fade_out) {
    if (!audio_system_) return;

    audio_system_->StopMusic(fade_out);
    race_music_playing_ = false;
}

void RaceAudio::SetAmbientMusic(const std::string& music_id) {
    if (!audio_system_) return;

    audio_system_->PlayMusic(music_id, music_volume_ * 0.5f, false);
}

void RaceAudio::SetCalloutVolume(float volume) {
    callout_volume_ = std::max(0.0f, std::min(1.0f, volume));
}

void RaceAudio::SetMusicVolume(float volume) {
    music_volume_ = std::max(0.0f, std::min(1.0f, volume));
    if (audio_system_ && race_music_playing_) {
        auto source = audio_system_->GetAudioSource(race_music_id_);
        if (source) {
            source->volume = music_volume_;
        }
    }
}

void RaceAudio::SetEffectVolume(float volume) {
    effect_volume_ = std::max(0.0f, std::min(1.0f, volume));
}

void RaceAudio::Update(float deltaTime) {
    phase_duration_ += deltaTime;
    ProcessCalloutQueue(deltaTime);
    UpdateCalloutCooldowns(deltaTime);
}

void RaceAudio::ProcessCalloutQueue(float deltaTime) {
    if (!callout_queue_.empty() && CanPlayCalloutType(callout_queue_.front().type)) {
        PlayNextCallout();
    }
}

void RaceAudio::PlayNextCallout() {
    if (callout_queue_.empty()) return;

    const auto& callout = callout_queue_.front();
    // In real implementation, this would use TTS or pre-recorded audio
    // For now, we just play a sound effect
    if (audio_system_) {
        audio_system_->PlaySound(warning_sound_id_, callout_volume_, 1.0f);
    }
    callout_queue_.erase(callout_queue_.begin());
}

void RaceAudio::UpdateCalloutCooldowns(float deltaTime) {
    for (auto& cooldown : callout_cooldowns_) {
        cooldown.remaining_time -= deltaTime;
    }

    auto it = callout_cooldowns_.begin();
    while (it != callout_cooldowns_.end()) {
        if (it->remaining_time <= 0.0f) {
            it = callout_cooldowns_.erase(it);
        } else {
            ++it;
        }
    }
}

bool RaceAudio::CanPlayCalloutType(CalloutType type) const {
    for (const auto& cooldown : callout_cooldowns_) {
        if (cooldown.type == type && cooldown.remaining_time > 0.0f) {
            return false;
        }
    }
    return true;
}

void RaceAudio::SetCalloutCooldown(CalloutType type, float cooldown) {
    CalloutCooldown cd;
    cd.type = type;
    cd.remaining_time = cooldown;
    callout_cooldowns_.push_back(cd);
}

std::string RaceAudio::GetDebugInfo() const {
    std::stringstream ss;
    ss << "RaceAudio Debug Info\n";
    ss << "- Race Phase: ";
    switch (current_phase_) {
        case RacePhase::PreRace:
            ss << "Pre-Race";
            break;
        case RacePhase::Countdown:
            ss << "Countdown";
            break;
        case RacePhase::RaceStart:
            ss << "Race Start";
            break;
        case RacePhase::Racing:
            ss << "Racing";
            break;
        case RacePhase::Finished:
            ss << "Finished";
            break;
    }
    ss << "\n";
    ss << "- Phase Duration: " << std::fixed << std::setprecision(2) << phase_duration_ << "s\n";
    ss << "- Callout Volume: " << callout_volume_ << "\n";
    ss << "- Music Volume: " << music_volume_ << "\n";
    ss << "- Effect Volume: " << effect_volume_ << "\n";
    ss << "- Pending Callouts: " << callout_queue_.size() << "\n";
    ss << "- Cooldowns Active: " << callout_cooldowns_.size() << "\n";
    ss << "- Race Music Playing: " << (race_music_playing_ ? "Yes" : "No") << "\n";
    return ss.str();
}
