#include "RaceUI.h"
#include "../game/RaceManager.h"
#include "../game/Vehicle.h"
#include "../input/RacingInput.h"
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

RaceUI::RaceUI(RaceManager* race_manager, Vehicle* vehicle,
               RacingInput* racing_input, UISystem* ui_system)
    : race_manager_(race_manager), vehicle_(vehicle),
      racing_input_(racing_input), ui_system_(ui_system) {
}

RaceUI::~RaceUI() {
    Shutdown();
}

bool RaceUI::Initialize() {
    if (!vehicle_ || !ui_system_ || !racing_input_) {
        return false;
    }

    // Initialize with default settings
    current_gear_ = 0;
    max_gear_ = 6;
    current_speed_ = 0.0f;
    current_health_ = 100.0f;
    throttle_input_ = 0.0f;
    brake_input_ = 0.0f;
    steering_input_ = 0.0f;
    shift_indicator_state_ = ShiftIndicatorState::Normal;

    initialized_ = true;
    return true;
}

void RaceUI::Shutdown() {
    initialized_ = false;
}

void RaceUI::Update(float deltaTime) {
    if (!initialized_) return;

    UpdateGearChangeCooldown(deltaTime);
    UpdateHUD(deltaTime);
    UpdateShiftIndicator(deltaTime);
    UpdateNotifications(deltaTime);
}

void RaceUI::SetTransmissionMode(TransmissionMode mode) {
    config_.transmission = mode;
    if (racing_input_) {
        racing_input_->SetAutomatic(mode == TransmissionMode::Automatic);
    }
}

void RaceUI::OnShiftUp() {
    if (!CanShiftGear() || !racing_input_) return;

    if (current_gear_ < max_gear_) {
        current_gear_++;
        racing_input_->ShiftUp();
        gear_change_cooldown_ = GEAR_CHANGE_COOLDOWN;
        shift_indicator_state_ = ShiftIndicatorState::Normal;
    }
}

void RaceUI::OnShiftDown() {
    if (!CanShiftGear() || !racing_input_) return;

    if (current_gear_ > 1) {
        current_gear_--;
        racing_input_->ShiftDown();
        gear_change_cooldown_ = GEAR_CHANGE_COOLDOWN;
        shift_indicator_state_ = ShiftIndicatorState::Normal;
    }
}

void RaceUI::UpdateHUD(float deltaTime) {
    if (!ui_system_) return;

    UpdateSpeedometer();
    UpdateGearIndicator();
    UpdateLapDisplay();
    UpdatePositionDisplay();
    UpdateDamageIndicator();
    UpdateInputBars();
}

void RaceUI::UpdateSpeedometer() {
    if (!vehicle_ || !ui_system_) return;

    current_speed_ = vehicle_->GetSpeed();
    ui_system_->UpdateSpeedometer(current_speed_, vehicle_->GetMaxSpeed());
}

void RaceUI::UpdateGearIndicator() {
    if (!ui_system_) return;

    ui_system_->UpdateGearIndicator(current_gear_, max_gear_,
                                    config_.transmission == TransmissionMode::Automatic);
}

void RaceUI::UpdateLapDisplay() {
    if (!race_manager_ || !ui_system_) return;

    auto racers = race_manager_->GetAllRacerInfoSorted();
    for (const auto& racer : racers) {
        if (racer.vehicle == vehicle_) {
            ui_system_->UpdateLapCounter(racer.current_lap, race_manager_->GetTotalLaps());
            ui_system_->UpdateLapTime(racer.total_race_time, racer.best_lap_time);
            break;
        }
    }
}

void RaceUI::UpdatePositionDisplay() {
    if (!race_manager_ || !ui_system_) return;

    auto racers = race_manager_->GetAllRacerInfoSorted();
    int player_position = 1;
    for (const auto& racer : racers) {
        if (racer.vehicle == vehicle_) {
            player_position = racer.position;
            break;
        }
    }

    ui_system_->UpdatePositionDisplay(player_position, racers.size());
}

void RaceUI::UpdateDamageIndicator() {
    if (!vehicle_ || !ui_system_) return;

    current_health_ = vehicle_->GetHealth();
    ui_system_->UpdateDamageIndicator(current_health_);
}

void RaceUI::UpdateInputBars() {
    if (!racing_input_ || !ui_system_) return;

    throttle_input_ = racing_input_->GetCurrentThrottle();
    brake_input_ = racing_input_->GetCurrentBrake();
    steering_input_ = racing_input_->GetCurrentSteering();

    ui_system_->UpdateThrottleBar(throttle_input_);
    ui_system_->UpdateBrakeBar(brake_input_);
    ui_system_->UpdateSteeringIndicator(steering_input_);
}

void RaceUI::UpdateShiftIndicator(float deltaTime) {
    shift_indicator_state_ = CalculateShiftIndicatorState();

    switch (shift_indicator_state_) {
        case ShiftIndicatorState::Redline:
            // Pulsing red effect
            blink_timer_ += deltaTime;
            if (blink_timer_ > (1.0f / blink_frequency_)) {
                blink_timer_ = 0.0f;
            }
            shift_indicator_intensity_ = 0.5f + 0.5f * std::sin(blink_timer_ * blink_frequency_ * 2.0f * 3.14159f);
            break;

        case ShiftIndicatorState::ShiftUp:
            shift_indicator_intensity_ = 0.8f;
            break;

        case ShiftIndicatorState::ShiftDown:
            shift_indicator_intensity_ = 0.6f;
            break;

        case ShiftIndicatorState::Blinking:
            blink_timer_ += deltaTime;
            if (blink_timer_ > (1.0f / blink_frequency_)) {
                blink_timer_ = 0.0f;
            }
            shift_indicator_intensity_ = 0.5f + 0.5f * std::sin(blink_timer_ * blink_frequency_ * 2.0f * 3.14159f);
            break;

        case ShiftIndicatorState::Normal:
            shift_indicator_intensity_ = 0.2f;  // Subtle glow
            break;
    }
}

ShiftIndicatorState RaceUI::CalculateShiftIndicatorState() const {
    if (!vehicle_) return ShiftIndicatorState::Normal;

    // Get engine RPM percentage (normalized 0-1)
    // In a real implementation, this would come from vehicle physics
    float speed_ratio = std::min(current_speed_ / vehicle_->GetMaxSpeed(), 1.0f);

    if (config_.transmission == TransmissionMode::Manual) {
        // Suggest shifts based on RPM
        if (speed_ratio > 0.95f) {
            return ShiftIndicatorState::Redline;  // At redline, suggest shift up
        } else if (speed_ratio > redline_threshold_) {
            return ShiftIndicatorState::ShiftUp;
        } else if (speed_ratio < 0.3f && current_gear_ > 1) {
            return ShiftIndicatorState::ShiftDown;
        }
    }

    return ShiftIndicatorState::Normal;
}

bool RaceUI::CanShiftGear() const {
    if (config_.transmission != TransmissionMode::Manual) {
        return false;  // Only manual transmission can shift
    }
    return gear_change_cooldown_ <= 0.0f;
}

void RaceUI::UpdateGearChangeCooldown(float deltaTime) {
    gear_change_cooldown_ -= deltaTime;
    gear_change_cooldown_ = std::max(0.0f, gear_change_cooldown_);
}

void RaceUI::UpdateNotifications(float deltaTime) {
    if (!notification_active_) return;

    notification_time_ += deltaTime;
    if (notification_time_ >= notification_duration_) {
        notification_active_ = false;
        notification_time_ = 0.0f;
    }
}

void RaceUI::DisplayCountdown(int seconds_remaining) {
    std::stringstream ss;
    if (seconds_remaining > 0) {
        ss << seconds_remaining;
    } else {
        ss << "GO!";
    }
    DisplayNotification(ss.str(), 1.0f);
}

void RaceUI::DisplayRaceStart() {
    DisplayNotification("RACE STARTED!", 2.0f);
}

void RaceUI::DisplayRaceFinish(int final_position, float race_time) {
    std::stringstream ss;
    ss << "FINISHED - Position " << final_position << " - Time ";
    ss << std::fixed << std::setprecision(2) << race_time << "s";
    DisplayNotification(ss.str(), 5.0f);
}

void RaceUI::DisplayLapComplete(int lap_number, float lap_time) {
    std::stringstream ss;
    ss << "Lap " << lap_number << " - " << std::fixed << std::setprecision(2) << lap_time << "s";
    DisplayNotification(ss.str(), 3.0f);
}

void RaceUI::DisplayPositionChange(int new_position) {
    std::stringstream ss;
    ss << "Position " << new_position;
    DisplayNotification(ss.str(), 2.5f);
}

void RaceUI::ShowHUD() {
    if (ui_system_) {
        ui_system_->ShowHUD();
    }
}

void RaceUI::HideHUD() {
    if (ui_system_) {
        ui_system_->HideHUD();
    }
}

void RaceUI::ToggleHUD() {
    if (ui_system_) {
        if (ui_system_->IsHUDVisible()) {
            HideHUD();
        } else {
            ShowHUD();
        }
    }
}

bool RaceUI::IsHUDVisible() const {
    if (ui_system_) {
        return ui_system_->IsHUDVisible();
    }
    return false;
}

void RaceUI::DisplayNotification(const std::string& text, float duration) {
    notification_text_ = text;
    notification_duration_ = duration;
    notification_time_ = 0.0f;
    notification_active_ = true;
}

std::string RaceUI::FormatTime(float seconds) const {
    int minutes = static_cast<int>(seconds) / 60;
    int secs = static_cast<int>(seconds) % 60;
    int millis = static_cast<int>((seconds - std::floor(seconds)) * 1000);

    std::stringstream ss;
    ss << minutes << ":" << std::setfill('0') << std::setw(2) << secs
       << "." << std::setw(3) << millis;
    return ss.str();
}

std::string RaceUI::FormatSpeed(float speed) const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(0) << speed << " km/h";
    return ss.str();
}

std::string RaceUI::GetDebugInfo() const {
    std::stringstream ss;
    ss << "RaceUI Debug Info\n";
    ss << "- Initialized: " << (initialized_ ? "Yes" : "No") << "\n";
    ss << "- Transmission: " << (config_.transmission == TransmissionMode::Automatic ? "Automatic" : "Manual") << "\n";
    ss << "- Current Gear: " << current_gear_ << " / " << max_gear_ << "\n";
    ss << "- Speed: " << std::fixed << std::setprecision(1) << current_speed_ << " km/h\n";
    ss << "- Health: " << std::setprecision(0) << current_health_ << "%\n";
    ss << "- Shift State: ";
    switch (shift_indicator_state_) {
        case ShiftIndicatorState::Normal:
            ss << "Normal";
            break;
        case ShiftIndicatorState::ShiftUp:
            ss << "Shift Up";
            break;
        case ShiftIndicatorState::ShiftDown:
            ss << "Shift Down";
            break;
        case ShiftIndicatorState::Redline:
            ss << "Redline";
            break;
        case ShiftIndicatorState::Blinking:
            ss << "Blinking";
            break;
    }
    ss << "\n";
    ss << "- Shift Intensity: " << std::setprecision(2) << shift_indicator_intensity_ << "\n";
    ss << "- Gear Cooldown: " << gear_change_cooldown_ << "s\n";
    ss << "- Throttle: " << std::setprecision(1) << throttle_input_ * 100.0f << "%\n";
    ss << "- Brake: " << brake_input_ * 100.0f << "%\n";
    ss << "- Steering: " << std::setprecision(2) << steering_input_ << "\n";
    return ss.str();
}
