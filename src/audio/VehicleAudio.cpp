#include "VehicleAudio.h"
#include "../game/Vehicle.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

VehicleAudio::VehicleAudio(Vehicle* vehicle, AudioSystem* audio_system, const std::string& vehicle_name)
    : vehicle_(vehicle), audio_system_(audio_system), vehicle_name_(vehicle_name) {
    engine_sound_id_ = "audio_engine_" + vehicle_name;
    idle_sound_id_ = "audio_idle_" + vehicle_name;
    brake_sound_id_ = "audio_brake_" + vehicle_name;
    crash_sound_id_ = "audio_crash_" + vehicle_name;
    skid_sound_id_ = "audio_skid_" + vehicle_name;
    horn_sound_id_ = "audio_horn_" + vehicle_name;
    gear_change_sound_id_ = "audio_gear_" + vehicle_name;
    start_sound_id_ = "audio_start_" + vehicle_name;
    stall_sound_id_ = "audio_stall_" + vehicle_name;
    road_sound_id_ = "audio_road_asphalt";
}

VehicleAudio::~VehicleAudio() {
    Shutdown();
}

bool VehicleAudio::Initialize() {
    if (!audio_system_) {
        return false;
    }

    LoadEngineSounds();
    LoadEffectSounds();
    LoadRoadSounds();

    return true;
}

void VehicleAudio::Shutdown() {
    StopEngine();
    if (audio_system_) {
        audio_system_->StopSound(engine_sound_id_);
        audio_system_->StopSound(brake_sound_id_);
        audio_system_->StopSound(skid_sound_id_);
        audio_system_->StopSound(road_sound_id_);
    }
}

void VehicleAudio::LoadEngineSounds() {
    if (!audio_system_) return;

    // Load engine sounds
    audio_system_->LoadAudio(engine_sound_id_, "assets/audio/engine/engine_loop.wav",
                            AudioSystem::AudioType::Ambient, AudioSystem::SoundCategory::Engine);
    audio_system_->LoadAudio(idle_sound_id_, "assets/audio/engine/idle.wav",
                            AudioSystem::AudioType::Ambient, AudioSystem::SoundCategory::Engine);
    audio_system_->LoadAudio(start_sound_id_, "assets/audio/engine/engine_start.wav",
                            AudioSystem::AudioType::SoundEffect, AudioSystem::SoundCategory::Engine);
    audio_system_->LoadAudio(stall_sound_id_, "assets/audio/engine/engine_stall.wav",
                            AudioSystem::AudioType::SoundEffect, AudioSystem::SoundCategory::Engine);
}

void VehicleAudio::LoadEffectSounds() {
    if (!audio_system_) return;

    // Load effect sounds
    audio_system_->LoadAudio(brake_sound_id_, "assets/audio/effects/brake.wav",
                            AudioSystem::AudioType::SoundEffect, AudioSystem::SoundCategory::Road);
    audio_system_->LoadAudio(skid_sound_id_, "assets/audio/effects/skid.wav",
                            AudioSystem::AudioType::Ambient, AudioSystem::SoundCategory::Road);
    audio_system_->LoadAudio(crash_sound_id_, "assets/audio/effects/crash.wav",
                            AudioSystem::AudioType::SoundEffect, AudioSystem::SoundCategory::Crash);
    audio_system_->LoadAudio(horn_sound_id_, "assets/audio/effects/horn.wav",
                            AudioSystem::AudioType::SoundEffect, AudioSystem::SoundCategory::UI);
    audio_system_->LoadAudio(gear_change_sound_id_, "assets/audio/effects/gear_change.wav",
                            AudioSystem::AudioType::SoundEffect, AudioSystem::SoundCategory::Engine);
}

void VehicleAudio::LoadRoadSounds() {
    if (!audio_system_) return;

    // Load road surface sounds
    audio_system_->LoadAudio("audio_road_asphalt", "assets/audio/road/asphalt.wav",
                            AudioSystem::AudioType::Ambient, AudioSystem::SoundCategory::Road);
    audio_system_->LoadAudio("audio_road_gravel", "assets/audio/road/gravel.wav",
                            AudioSystem::AudioType::Ambient, AudioSystem::SoundCategory::Road);
    audio_system_->LoadAudio("audio_road_water", "assets/audio/road/water.wav",
                            AudioSystem::AudioType::Ambient, AudioSystem::SoundCategory::Road);
    audio_system_->LoadAudio("audio_road_sand", "assets/audio/road/sand.wav",
                            AudioSystem::AudioType::Ambient, AudioSystem::SoundCategory::Road);
    audio_system_->LoadAudio("audio_road_snow", "assets/audio/road/snow.wav",
                            AudioSystem::AudioType::Ambient, AudioSystem::SoundCategory::Road);
}

void VehicleAudio::StartEngine() {
    if (!audio_system_ || !vehicle_) return;

    engine_running_ = true;
    audio_system_->PlaySound(start_sound_id_, engine_volume_, 1.0f);
    audio_system_->PlaySound(engine_sound_id_, engine_volume_, 1.0f);
}

void VehicleAudio::StopEngine() {
    if (!audio_system_) return;

    engine_running_ = false;
    audio_system_->StopSound(engine_sound_id_);
}

void VehicleAudio::Update(float deltaTime) {
    if (!audio_system_ || !vehicle_ || !engine_running_) {
        return;
    }

    UpdateEngineSound(deltaTime);
    UpdateRoadSound();
}

void VehicleAudio::UpdateEngineSound(float deltaTime) {
    // Calculate frequency based on vehicle speed and throttle
    float frequency = CalculateEngineFrequency();
    float volume = CalculateEngineVolume();

    // Update engine sound frequency (pitch)
    if (audio_system_->IsAudioPlaying(engine_sound_id_)) {
        auto source = audio_system_->GetAudioSource(engine_sound_id_);
        if (source) {
            source->pitch = frequency;
            source->volume = volume;
        }
    }

    current_engine_frequency_ = frequency;
}

float VehicleAudio::CalculateEngineFrequency() const {
    if (!vehicle_) return 1.0f;

    float speed = vehicle_->GetSpeed();
    float max_speed = vehicle_->GetMaxSpeed();
    float throttle = 1.0f;  // Would be from input system

    // Engine frequency ranges from 0.5 (idle) to 2.5 (redline)
    float freq_idle = 0.5f;
    float freq_max = 2.5f;
    float speed_ratio = std::min(speed / max_speed, 1.0f);
    float frequency = freq_idle + (speed_ratio * (freq_max - freq_idle));

    return std::max(0.5f, std::min(2.5f, frequency));
}

float VehicleAudio::CalculateEngineVolume() const {
    if (!vehicle_) return 0.0f;

    float speed = vehicle_->GetSpeed();
    float max_speed = vehicle_->GetMaxSpeed();
    float speed_ratio = std::min(speed / max_speed, 1.0f);

    // Volume increases with speed
    float volume = engine_volume_ * (0.5f + (speed_ratio * 0.5f));
    return std::max(0.2f, std::min(engine_volume_, volume));
}

float VehicleAudio::CalculateRoadNoiseVolume() const {
    if (!vehicle_) return 0.0f;

    float speed = vehicle_->GetSpeed();
    float max_speed = vehicle_->GetMaxSpeed();
    float speed_ratio = std::min(speed / max_speed, 1.0f);

    // Road noise increases with speed
    return road_volume_ * speed_ratio;
}

void VehicleAudio::UpdateRoadSound() {
    float volume = CalculateRoadNoiseVolume();

    if (volume > 0.05f && !road_sound_playing_) {
        audio_system_->PlaySound(road_sound_id_, volume, road_frequency_);
        road_sound_playing_ = true;
    } else if (volume <= 0.05f && road_sound_playing_) {
        audio_system_->StopSound(road_sound_id_);
        road_sound_playing_ = false;
    } else if (road_sound_playing_) {
        auto source = audio_system_->GetAudioSource(road_sound_id_);
        if (source) {
            source->volume = volume;
        }
    }
}

void VehicleAudio::SetRoadSurfaceType(const std::string& surface_type) {
    current_surface_ = surface_type;
    road_sound_id_ = GetRoadSoundId(surface_type);
}

std::string VehicleAudio::GetRoadSoundId(const std::string& surface) const {
    if (surface == "asphalt") return "audio_road_asphalt";
    if (surface == "gravel") return "audio_road_gravel";
    if (surface == "water") return "audio_road_water";
    if (surface == "sand") return "audio_road_sand";
    if (surface == "snow") return "audio_road_snow";
    return "audio_road_asphalt";
}

void VehicleAudio::PlayCrashSound(float impact_force) {
    if (!audio_system_) return;

    // Scale volume by impact force
    float volume = std::min(1.0f, impact_force * 0.5f);
    audio_system_->PlaySound(crash_sound_id_, volume, 1.0f);
}

void VehicleAudio::PlayBrakeSound() {
    if (!audio_system_ || brake_sound_playing_) return;

    audio_system_->PlaySound(brake_sound_id_, sound_effect_volume_, 1.0f);
    brake_sound_playing_ = true;
}

void VehicleAudio::PlaySkidSound() {
    if (!audio_system_ || skid_sound_playing_) return;

    audio_system_->PlaySound(skid_sound_id_, sound_effect_volume_ * 0.7f, 1.2f);
    skid_sound_playing_ = true;
}

void VehicleAudio::PlayHornSound() {
    if (!audio_system_) return;

    audio_system_->PlaySound(horn_sound_id_, sound_effect_volume_, 1.0f);
}

void VehicleAudio::PlayGearChangeSound() {
    if (!audio_system_) return;

    audio_system_->PlaySound(gear_change_sound_id_, sound_effect_volume_ * 0.6f, 1.0f);
}

void VehicleAudio::PlayStartSound() {
    if (!audio_system_) return;

    audio_system_->PlaySound(start_sound_id_, engine_volume_, 1.0f);
}

void VehicleAudio::PlayStallSound() {
    if (!audio_system_) return;

    audio_system_->PlaySound(stall_sound_id_, engine_volume_, 1.2f);
}

void VehicleAudio::SetEngineVolume(float volume) {
    engine_volume_ = std::max(0.0f, std::min(1.0f, volume));
}

void VehicleAudio::SetSoundEffectVolume(float volume) {
    sound_effect_volume_ = std::max(0.0f, std::min(1.0f, volume));
}

std::string VehicleAudio::GetDebugInfo() const {
    std::stringstream ss;
    ss << "VehicleAudio Debug Info (" << vehicle_name_ << ")\n";
    ss << "- Engine Running: " << (engine_running_ ? "Yes" : "No") << "\n";
    ss << "- Engine Volume: " << std::fixed << std::setprecision(2) << engine_volume_ << "\n";
    ss << "- Engine Frequency: " << current_engine_frequency_ << "\n";
    ss << "- Road Surface: " << current_surface_ << "\n";
    ss << "- Road Sound Playing: " << (road_sound_playing_ ? "Yes" : "No") << "\n";
    ss << "- Road Volume: " << road_volume_ << "\n";
    return ss.str();
}
