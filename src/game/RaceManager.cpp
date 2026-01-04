#include "RaceManager.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

RaceManager::RaceManager()
    : GameManager() {
}

RaceManager::~RaceManager() {
    Shutdown();
}

bool RaceManager::InitializeRace(RaceType race_type, int num_laps, float track_length) {
    if (!Initialize()) {
        return false;
    }

    race_type_ = race_type;
    total_laps_ = num_laps;
    track_length_ = track_length;
    current_lap_ = 1;
    race_elapsed_time_ = 0.0f;
    countdown_timer_ = 0.0f;
    race_status_ = RaceStatus::NotStarted;

    // Initialize player info
    player_info_.position = 1;
    player_info_.current_lap = 1;
    player_info_.lap_distance = 0.0f;
    player_info_.total_distance = 0.0f;
    player_info_.best_lap_time = 0.0f;
    player_info_.current_lap_start_time = 0.0f;
    player_info_.total_race_time = 0.0f;
    player_info_.finished = false;
    player_info_.finish_time = 0.0f;

    return true;
}

void RaceManager::StartRace() {
    race_status_ = RaceStatus::CountdownWarning;
    countdown_timer_ = countdown_time_;
    race_elapsed_time_ = 0.0f;
}

void RaceManager::FinishRace() {
    race_status_ = RaceStatus::Finished;
    if (player_vehicle_) {
        player_vehicle_->StopEngine();
    }
}

void RaceManager::AbandonRace() {
    race_status_ = RaceStatus::Abandoned;
    if (player_vehicle_) {
        player_vehicle_->StopEngine();
    }
}

void RaceManager::SetPlayerVehicle(std::shared_ptr<Vehicle> vehicle) {
    if (player_vehicle_) {
        player_vehicle_->Shutdown();
    }
    player_vehicle_ = vehicle;
    if (player_vehicle_) {
        player_vehicle_->Initialize();
        player_info_.vehicle = vehicle;
    }
}

void RaceManager::AddOpponentVehicle(std::shared_ptr<Vehicle> vehicle) {
    if (!vehicle) return;

    vehicle->Initialize();
    opponent_vehicles_.push_back(vehicle);

    // Initialize opponent info
    RacerInfo info;
    info.vehicle = vehicle;
    info.position = opponent_vehicles_.size() + 1;
    info.current_lap = 1;
    info.lap_distance = 0.0f;
    info.total_distance = 0.0f;
    opponent_info_[vehicle] = info;
}

void RaceManager::RemoveOpponentVehicle(std::shared_ptr<Vehicle> vehicle) {
    auto it = std::find(opponent_vehicles_.begin(), opponent_vehicles_.end(), vehicle);
    if (it != opponent_vehicles_.end()) {
        (*it)->Shutdown();
        opponent_vehicles_.erase(it);
        opponent_info_.erase(vehicle);
    }
}

RaceManager::RacerInfo RaceManager::GetRacerInfo(std::shared_ptr<Vehicle> vehicle) const {
    if (vehicle == player_vehicle_) {
        return player_info_;
    }

    auto it = opponent_info_.find(vehicle);
    if (it != opponent_info_.end()) {
        return it->second;
    }

    return RacerInfo();  // Return empty info
}

std::vector<RaceManager::RacerInfo> RaceManager::GetAllRacerInfoSorted() const {
    std::vector<RacerInfo> all_racers;

    // Add player
    all_racers.push_back(player_info_);

    // Add opponents
    for (const auto& opponent : opponent_vehicles_) {
        auto it = opponent_info_.find(opponent);
        if (it != opponent_info_.end()) {
            all_racers.push_back(it->second);
        }
    }

    // Sort by distance (descending - more distance = higher position)
    std::sort(all_racers.begin(), all_racers.end(),
              [](const RacerInfo& a, const RacerInfo& b) {
                  // First sort by lap
                  if (a.current_lap != b.current_lap) {
                      return a.current_lap > b.current_lap;
                  }
                  // Then by distance on current lap
                  return a.lap_distance > b.lap_distance;
              });

    // Update positions
    for (size_t i = 0; i < all_racers.size(); ++i) {
        all_racers[i].position = i + 1;
    }

    return all_racers;
}

bool RaceManager::IsLapComplete(std::shared_ptr<Vehicle> vehicle, float vehicle_distance) const {
    float lap_progress = std::fmod(vehicle_distance, track_length_);
    return lap_progress >= track_length_ * 0.95f;  // Within 95% to 100%
}

void RaceManager::CompleteLap(std::shared_ptr<Vehicle> vehicle, float lap_time) {
    if (vehicle == player_vehicle_) {
        player_info_.current_lap++;
        player_info_.lap_distance = 0.0f;

        // Track best lap
        if (player_info_.best_lap_time == 0.0f || lap_time < player_info_.best_lap_time) {
            player_info_.best_lap_time = lap_time;
        }
        player_info_.current_lap_start_time = race_elapsed_time_;
    } else {
        auto it = opponent_info_.find(vehicle);
        if (it != opponent_info_.end()) {
            it->second.current_lap++;
            it->second.lap_distance = 0.0f;

            if (it->second.best_lap_time == 0.0f || lap_time < it->second.best_lap_time) {
                it->second.best_lap_time = lap_time;
            }
            it->second.current_lap_start_time = race_elapsed_time_;
        }
    }
}

void RaceManager::UpdatePositions() {
    auto sorted = GetAllRacerInfoSorted();

    // Update player position
    for (const auto& info : sorted) {
        if (info.vehicle == player_vehicle_) {
            player_info_.position = info.position;
            break;
        }
    }

    // Update opponent positions
    for (const auto& info : sorted) {
        if (opponent_info_.find(info.vehicle) != opponent_info_.end()) {
            opponent_info_[info.vehicle].position = info.position;
        }
    }
}

bool RaceManager::CheckPlayerFinished() const {
    if (!player_vehicle_) return false;
    return player_info_.current_lap > total_laps_ && player_info_.lap_distance >= track_length_;
}

bool RaceManager::CheckAllFinished() const {
    if (!CheckPlayerFinished()) return false;

    // Check if all opponents have finished
    for (const auto& opponent : opponent_vehicles_) {
        auto it = opponent_info_.find(opponent);
        if (it != opponent_info_.end()) {
            if (!it->second.finished) {
                return false;
            }
        }
    }

    return true;
}

void RaceManager::HandleVehicleCrash(std::shared_ptr<Vehicle> vehicle) {
    if (!vehicle) return;

    // Vehicle already handles crash state
    // This is for race-specific crash logic
    if (vehicle == player_vehicle_) {
        // Could add penalties, position changes, etc.
    }
}

void RaceManager::Update(float deltaTime) {
    if (race_status_ == RaceStatus::NotStarted) {
        return;
    }

    // Update countdown
    if (IsCountingDown()) {
        UpdateCountdown(deltaTime);
    }

    // Update race time
    if (race_status_ == RaceStatus::Racing) {
        UpdateRaceTime(deltaTime);
        UpdateVehicleDistances();
        CalculatePositions();
        CheckLapCompletion();
    }

    // Call parent update
    GameManager::Update(deltaTime);
}

void RaceManager::UpdateCountdown(float deltaTime) {
    countdown_timer_ -= deltaTime;

    if (countdown_timer_ <= 0.0f) {
        if (race_status_ == RaceStatus::CountdownWarning) {
            race_status_ = RaceStatus::CountdownPrepare;
            countdown_timer_ = 1.0f;
        } else if (race_status_ == RaceStatus::CountdownPrepare) {
            race_status_ = RaceStatus::CountdownGo;
            countdown_timer_ = 0.5f;
        } else if (race_status_ == RaceStatus::CountdownGo) {
            race_status_ = RaceStatus::Racing;
            if (player_vehicle_) {
                player_vehicle_->StartEngine();
            }
            for (auto& opponent : opponent_vehicles_) {
                opponent->StartEngine();
            }
        }
    }
}

void RaceManager::UpdateRaceTime(float deltaTime) {
    race_elapsed_time_ += deltaTime;
    player_info_.total_race_time = race_elapsed_time_;

    // Update opponent times
    for (auto& opponent : opponent_vehicles_) {
        auto it = opponent_info_.find(opponent);
        if (it != opponent_info_.end()) {
            it->second.total_race_time = race_elapsed_time_;
        }
    }
}

void RaceManager::UpdateVehicleDistances() {
    if (player_vehicle_) {
        float distance = player_vehicle_->GetDistanceTraveled();
        player_info_.total_distance = distance;
        player_info_.lap_distance = std::fmod(distance, track_length_);
        player_info_.current_lap = static_cast<int>(distance / track_length_) + 1;
    }

    for (auto& opponent : opponent_vehicles_) {
        float distance = opponent->GetDistanceTraveled();
        auto it = opponent_info_.find(opponent);
        if (it != opponent_info_.end()) {
            it->second.total_distance = distance;
            it->second.lap_distance = std::fmod(distance, track_length_);
            it->second.current_lap = static_cast<int>(distance / track_length_) + 1;
        }
    }
}

void RaceManager::CalculatePositions() {
    UpdatePositions();
}

void RaceManager::CheckLapCompletion() {
    // Check player lap completion
    if (player_vehicle_ && IsLapComplete(player_vehicle_, player_info_.total_distance)) {
        float lap_time = race_elapsed_time_ - player_info_.current_lap_start_time;
        CompleteLap(player_vehicle_, lap_time);
    }

    // Check opponent lap completions
    for (auto& opponent : opponent_vehicles_) {
        auto it = opponent_info_.find(opponent);
        if (it != opponent_info_.end()) {
            if (IsLapComplete(opponent, it->second.total_distance)) {
                float lap_time = race_elapsed_time_ - it->second.current_lap_start_time;
                CompleteLap(opponent, lap_time);
            }
        }
    }

    // Check finish condition
    if (CheckPlayerFinished()) {
        player_info_.finished = true;
        player_info_.finish_time = race_elapsed_time_;
        FinishRace();
    }
}

std::string RaceManager::GetRaceDebugInfo() const {
    std::stringstream ss;
    ss << "RaceManager Debug Info\n";
    ss << "- Race Type: ";
    switch (race_type_) {
        case RaceType::SingleRace:
            ss << "Single Race";
            break;
        case RaceType::TimeTrial:
            ss << "Time Trial";
            break;
        case RaceType::Multiplayer:
            ss << "Multiplayer";
            break;
    }
    ss << "\n";
    ss << "- Race Status: ";
    switch (race_status_) {
        case RaceStatus::NotStarted:
            ss << "Not Started";
            break;
        case RaceStatus::CountdownWarning:
            ss << "Countdown Warning";
            break;
        case RaceStatus::CountdownPrepare:
            ss << "Countdown Prepare";
            break;
        case RaceStatus::CountdownGo:
            ss << "Countdown Go";
            break;
        case RaceStatus::Racing:
            ss << "Racing";
            break;
        case RaceStatus::Finished:
            ss << "Finished";
            break;
        case RaceStatus::Crashed:
            ss << "Crashed";
            break;
        case RaceStatus::Abandoned:
            ss << "Abandoned";
            break;
    }
    ss << "\n";
    ss << "- Elapsed Time: " << std::fixed << std::setprecision(2) << race_elapsed_time_ << "s\n";
    ss << "- Countdown: " << countdown_timer_ << "s\n";
    ss << "- Current Lap: " << current_lap_ << " / " << total_laps_ << "\n";
    ss << "- Track Length: " << track_length_ << " units\n";
    ss << "- Player Position: " << player_info_.position << "\n";
    ss << "- Opponents: " << opponent_vehicles_.size() << "\n";
    return ss.str();
}

std::string RaceManager::GetPositionsString() const {
    auto sorted = GetAllRacerInfoSorted();
    std::stringstream ss;
    ss << "\n=== RACE POSITIONS ===\n";
    for (const auto& racer : sorted) {
        ss << "P" << racer.position << ": "
           << "Lap " << racer.current_lap << " "
           << "(" << std::fixed << std::setprecision(1) << racer.lap_distance << "m) "
           << "Best: " << std::setprecision(2) << racer.best_lap_time << "s\n";
    }
    return ss.str();
}
