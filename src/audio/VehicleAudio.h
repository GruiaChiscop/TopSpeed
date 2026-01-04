#pragma once

#include "AudioSystem.h"
#include <memory>
#include <string>

class Vehicle;  // Forward declaration

/**
 * @class VehicleAudio
 * @brief Manages audio for a single vehicle (engine sounds, crashes, etc.)
 * 
 * Handles dynamic engine sound based on speed and throttle,
 * crash sounds, skid sounds, and other vehicle-specific audio.
 */
class VehicleAudio {
public:
    VehicleAudio(Vehicle* vehicle, AudioSystem* audio_system, const std::string& vehicle_name);
    ~VehicleAudio();

    // Initialization
    bool Initialize();
    void Shutdown();

    // Update (call from game loop)
    void Update(float deltaTime);

    // Engine sounds
    void StartEngine();
    void StopEngine();
    bool IsEngineRunning() const { return engine_running_; }

    // Event sounds
    void PlayCrashSound(float impact_force = 1.0f);
    void PlayBrakeSound();
    void PlaySkidSound();
    void PlayHornSound();
    void PlayGearChangeSound();
    void PlayStartSound();
    void PlayStallSound();

    // Volume control
    void SetEngineVolume(float volume);
    void SetSoundEffectVolume(float volume);
    float GetEngineVolume() const { return engine_volume_; }

    // Engine audio management
    void UpdateEngineSound(float deltaTime);
    void UpdateEngineFrequency();

    // Road surface audio
    void SetRoadSurfaceType(const std::string& surface_type);
    void UpdateRoadSound();

    // Debug
    std::string GetDebugInfo() const;

private:
    Vehicle* vehicle_;  // Reference to vehicle
    AudioSystem* audio_system_;  // Reference to audio system
    std::string vehicle_name_;

    // Engine state
    bool engine_running_ = false;
    float engine_volume_ = 0.8f;
    float sound_effect_volume_ = 1.0f;
    float current_engine_frequency_ = 1.0f;  // Pitch multiplier
    float engine_frequency_timer_ = 0.0f;

    // Sound IDs
    std::string engine_sound_id_;
    std::string idle_sound_id_;
    std::string brake_sound_id_;
    std::string crash_sound_id_;
    std::string skid_sound_id_;
    std::string horn_sound_id_;
    std::string gear_change_sound_id_;
    std::string start_sound_id_;
    std::string stall_sound_id_;
    std::string road_sound_id_;  // Surface-dependent

    // Road surface
    std::string current_surface_ = "asphalt";
    float road_volume_ = 0.5f;
    float road_frequency_ = 1.0f;

    // Sound state tracking
    bool brake_sound_playing_ = false;
    bool skid_sound_playing_ = false;
    bool road_sound_playing_ = false;

    // Helper methods
    void LoadEngineSounds();
    void LoadEffectSounds();
    void LoadRoadSounds();
    float CalculateEngineFrequency() const;
    float CalculateEngineVolume() const;
    float CalculateRoadNoiseVolume() const;
    std::string GetRoadSoundId(const std::string& surface) const;
};
