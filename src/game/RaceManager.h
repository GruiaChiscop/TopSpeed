#pragma once

#include "GameManager.h"
#include "Vehicle.h"
#include <memory>
#include <vector>

/**
 * @class RaceManager
 * @brief Racing-specific game manager that extends GameManager
 * 
 * Manages race-specific logic including lap tracking, finishing, positioning,
 * and opponent management. Inherits core game management from GameManager.
 */
class RaceManager : public GameManager {
public:
    enum class RaceType {
        SingleRace,      // Race vs AI opponents
        TimeTrial,       // Solo speed run
        Multiplayer      // Race vs human players
    };

    enum class RaceStatus {
        NotStarted,
        CountdownWarning,
        CountdownPrepare,
        CountdownGo,
        Racing,
        Finished,
        Crashed,
        Abandoned
    };

    struct RacerInfo {
        std::shared_ptr<Vehicle> vehicle;
        int position = 0;           // 1st, 2nd, 3rd, etc.
        int current_lap = 0;
        float lap_distance = 0.0f;  // Distance on current lap
        float total_distance = 0.0f;
        float best_lap_time = 0.0f;
        float current_lap_start_time = 0.0f;
        float total_race_time = 0.0f;
        bool finished = false;
        float finish_time = 0.0f;
    };

    RaceManager();
    ~RaceManager();

    // Lifecycle
    bool InitializeRace(RaceType race_type, int num_laps = 1, float track_length = 1000.0f);
    void StartRace();
    void FinishRace();
    void AbandonRace();

    // Race state
    RaceStatus GetRaceStatus() const { return race_status_; }
    RaceType GetRaceType() const { return race_type_; }
    bool IsRaceActive() const { return race_status_ == RaceStatus::Racing; }
    bool IsRaceFinished() const { return race_status_ == RaceStatus::Finished; }

    // Player vehicle management
    void SetPlayerVehicle(std::shared_ptr<Vehicle> vehicle);
    std::shared_ptr<Vehicle> GetPlayerVehicle() const { return player_vehicle_; }

    // Opponent management
    void AddOpponentVehicle(std::shared_ptr<Vehicle> vehicle);
    void RemoveOpponentVehicle(std::shared_ptr<Vehicle> vehicle);
    std::vector<std::shared_ptr<Vehicle>> GetOpponentVehicles() const { return opponent_vehicles_; }
    int GetOpponentCount() const { return opponent_vehicles_.size(); }

    // Racer information
    RacerInfo GetPlayerRacerInfo() const { return player_info_; }
    RacerInfo GetRacerInfo(std::shared_ptr<Vehicle> vehicle) const;
    std::vector<RacerInfo> GetAllRacerInfoSorted() const;  // Sorted by position

    // Lap tracking
    int GetCurrentLap() const { return current_lap_; }
    int GetTotalLaps() const { return total_laps_; }
    bool IsLapComplete(std::shared_ptr<Vehicle> vehicle, float vehicle_distance) const;
    void CompleteLap(std::shared_ptr<Vehicle> vehicle, float lap_time);

    // Track setup
    void SetTrackLength(float length) { track_length_ = length; }
    float GetTrackLength() const { return track_length_; }
    void SetTotalLaps(int laps) { total_laps_ = laps; }

    // Race timing
    float GetRaceElapsedTime() const { return race_elapsed_time_; }
    float GetCountdownTime() const { return countdown_time_; }
    bool IsCountingDown() const { return race_status_ >= RaceStatus::CountdownWarning && 
                                         race_status_ <= RaceStatus::CountdownGo; }

    // Positioning and ranking
    void UpdatePositions();
    int GetPlayerPosition() const { return player_info_.position; }
    int GetPlayerLap() const { return player_info_.current_lap; }
    void SetPlayerLapDistance(float distance) { player_info_.lap_distance = distance; }
    void SetPlayerTotalDistance(float distance) { player_info_.total_distance = distance; }

    // Finish detection
    bool CheckPlayerFinished() const;
    bool CheckAllFinished() const;

    // Crash handling
    void HandleVehicleCrash(std::shared_ptr<Vehicle> vehicle);

    // Update and rendering
    void Update(float deltaTime) override;

    // Debug
    std::string GetRaceDebugInfo() const;
    std::string GetPositionsString() const;

private:
    RaceType race_type_ = RaceType::SingleRace;
    RaceStatus race_status_ = RaceStatus::NotStarted;

    // Vehicles
    std::shared_ptr<Vehicle> player_vehicle_;
    std::vector<std::shared_ptr<Vehicle>> opponent_vehicles_;

    // Racer info
    RacerInfo player_info_;
    std::unordered_map<std::shared_ptr<Vehicle>, RacerInfo> opponent_info_;

    // Race configuration
    int total_laps_ = 1;
    float track_length_ = 1000.0f;  // Distance for one lap
    int current_lap_ = 1;

    // Race timing
    float race_elapsed_time_ = 0.0f;
    float countdown_time_ = 3.0f;  // Countdown before race starts (seconds)
    float countdown_timer_ = 0.0f;

    // Helper methods
    void UpdateCountdown(float deltaTime);
    void UpdateRaceTime(float deltaTime);
    void UpdateVehicleDistances();
    void CalculatePositions();
    void CheckLapCompletion();
};
