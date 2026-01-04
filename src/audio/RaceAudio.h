#pragma once

#include "AudioSystem.h"
#include <memory>
#include <string>
#include <vector>

class RaceManager;  // Forward declaration

/**
 * @class RaceAudio
 * @brief Manages audio for race events and ambient sounds
 * 
 * Handles countdown beeps, race start/finish sounds, background music,
 * and race callouts (road descriptions, position updates, etc.)
 */
class RaceAudio {
public:
    enum class RacePhase {
        PreRace,        // Before countdown
        Countdown,      // 3...2...1
        RaceStart,      // "Go!"
        Racing,         // During race
        Finished        // Race complete
    };

    enum class CalloutType {
        PositionChange,     // "You're in 2nd place"
        RoadDescription,    // "Hard left turn"
        SpeedWarning,       // "Slow down"
        ObstacleWarning,    // "Obstacle ahead" (obstacle tracks)
        LapComplete,        // "Lap 1 complete"
        LapTime,            // "1:23.456"
        Finished            // "You finished!"
    };

    struct Callout {
        CalloutType type;
        std::string text;  // What to speak
        float duration = 2.0f;
        float cooldown = 1.0f;  // Time before same type can play again
    };

    RaceAudio(RaceManager* race_manager, AudioSystem* audio_system);
    ~RaceAudio();

    // Initialization
    bool Initialize();
    void Shutdown();

    // Update (call from game loop)
    void Update(float deltaTime);

    // Race phase management
    void SetRacePhase(RacePhase phase);
    RacePhase GetRacePhase() const { return current_phase_; }

    // Countdown sounds
    void PlayCountdownBeep(int remaining_count);  // 3, 2, 1, GO

    // Race event sounds
    void PlayRaceStartSound();
    void PlayRaceFinishSound();
    void PlayPositionChangeSound();
    void PlayWarningSound();

    // Callout system (for road descriptions and announcements)
    void QueueCallout(const Callout& callout);
    void QueuePositionCallout(int position, const std::string& opponent_name = "");
    void QueueRoadCallout(const std::string& road_description);
    void QueueObstacleCallout(const std::string& obstacle_type);
    void QueueLapCallout(int lap_number, float lap_time = 0.0f);

    // Background music
    void SetRaceMusic(const std::string& music_id, bool fade_in = true);
    void StopRaceMusic(bool fade_out = true);
    void SetAmbientMusic(const std::string& music_id);

    // Volume control
    void SetCalloutVolume(float volume);
    void SetMusicVolume(float volume);
    void SetEffectVolume(float volume);
    float GetCalloutVolume() const { return callout_volume_; }

    // Audio asset preloading
    void PreloadRaceSounds();
    void PreloadRaceMusic();

    // Query
    bool IsCalloutQueueEmpty() const { return callout_queue_.empty(); }
    int GetPendingCallouts() const { return callout_queue_.size(); }

    // Debug
    std::string GetDebugInfo() const;

private:
    RaceManager* race_manager_;  // Reference to race manager
    AudioSystem* audio_system_;  // Reference to audio system

    RacePhase current_phase_ = RacePhase::PreRace;
    std::vector<Callout> callout_queue_;

    // Sound IDs
    std::string countdown_3_sound_id_ = "audio_countdown_3";
    std::string countdown_2_sound_id_ = "audio_countdown_2";
    std::string countdown_1_sound_id_ = "audio_countdown_1";
    std::string countdown_go_sound_id_ = "audio_countdown_go";
    std::string race_start_sound_id_ = "audio_race_start";
    std::string race_finish_sound_id_ = "audio_race_finish";
    std::string position_change_sound_id_ = "audio_position_change";
    std::string warning_sound_id_ = "audio_warning";
    std::string lap_complete_sound_id_ = "audio_lap_complete";

    // Music IDs
    std::string race_music_id_ = "music_race_main";
    std::string menu_music_id_ = "music_menu";
    std::string ambient_music_id_ = "music_ambient";

    // Volume levels
    float callout_volume_ = 0.9f;
    float music_volume_ = 0.6f;
    float effect_volume_ = 1.0f;

    // State tracking
    float callout_cooldown_timer_ = 0.0f;
    float phase_duration_ = 0.0f;
    bool race_music_playing_ = false;

    // Callout timing
    struct CalloutCooldown {
        CalloutType type;
        float remaining_time = 0.0f;
    };
    std::vector<CalloutCooldown> callout_cooldowns_;

    // Helper methods
    void ProcessCalloutQueue(float deltaTime);
    void PlayNextCallout();
    void UpdateCalloutCooldowns(float deltaTime);
    bool CanPlayCalloutType(CalloutType type) const;
    void SetCalloutCooldown(CalloutType type, float cooldown);
};
