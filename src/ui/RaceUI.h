#pragma once

#include "UISystem.h"
#include <memory>
#include <string>

class RaceManager;  // Forward declaration
class Vehicle;      // Forward declaration
class RacingInput;  // Forward declaration

/**
 * @class RaceUI
 * @brief Manages all UI for racing, including HUD, lap times, and manual gear indicator
 * 
 * Displays vehicle speed, gear, lap information, position, and race status.
 * Provides visual feedback for manual gear shifting with shift indicator.
 */
class RaceUI {
public:
    enum class TransmissionMode {
        Automatic,  // Automatic gear changes
        Manual      // Manual gear shifting (player controls)
    };

    enum class ShiftIndicatorState {
        Normal,     // Normal operation
        ShiftUp,    // Suggest upshift (RPM high)
        ShiftDown,  // Suggest downshift (RPM low)
        Redline,    // Engine at redline (visual warning)
        Blinking    // Blinking/pulsing effect
    };

    struct RaceUIConfig {
        bool show_speedometer = true;
        bool show_gear = true;
        bool show_lap_counter = true;
        bool show_position = true;
        bool show_lap_times = true;
        bool show_damage = true;
        bool show_bars = true;           // Throttle/brake/steering bars
        bool show_shift_indicator = true; // Manual gear shift hint
        TransmissionMode transmission = TransmissionMode::Automatic;
        float hud_opacity = 0.8f;
        float hud_scale = 1.0f;
    };

    RaceUI(RaceManager* race_manager, Vehicle* vehicle,
            RacingInput* racing_input, UISystem* ui_system);
    ~RaceUI();

    // Initialization
    bool Initialize();
    void Shutdown();

    // Update (call from game loop)
    void Update(float deltaTime);

    // Configuration
    void SetConfig(const RaceUIConfig& config) { config_ = config; }
    RaceUIConfig GetConfig() const { return config_; }

    // Transmission mode control
    void SetTransmissionMode(TransmissionMode mode);
    TransmissionMode GetTransmissionMode() const { return config_.transmission; }
    bool IsManualTransmission() const {
        return config_.transmission == TransmissionMode::Manual;
    }

    // Manual gear control (called by RacingInput)
    void OnShiftUp();
    void OnShiftDown();
    int GetCurrentGear() const { return current_gear_; }
    int GetMaxGear() const { return max_gear_; }

    // Shift indicator
    ShiftIndicatorState GetShiftIndicatorState() const { return shift_indicator_state_; }
    float GetShiftIndicatorIntensity() const { return shift_indicator_intensity_; }

    // HUD visibility
    void ShowHUD();
    void HideHUD();
    void ToggleHUD();
    bool IsHUDVisible() const;

    // Race phase display
    void DisplayCountdown(int seconds_remaining);
    void DisplayRaceStart();
    void DisplayRaceFinish(int final_position, float race_time);
    void DisplayLapComplete(int lap_number, float lap_time);
    void DisplayPositionChange(int new_position);

    // Debug
    std::string GetDebugInfo() const;

private:
    RaceManager* race_manager_;     // Reference to race manager
    Vehicle* vehicle_;              // Reference to player vehicle
    RacingInput* racing_input_;     // Reference to racing input
    UISystem* ui_system_;           // Reference to UI system

    RaceUIConfig config_;

    // Vehicle state tracking
    int current_gear_ = 0;
    int max_gear_ = 6;
    float current_speed_ = 0.0f;
    float current_health_ = 100.0f;
    float throttle_input_ = 0.0f;
    float brake_input_ = 0.0f;
    float steering_input_ = 0.0f;

    // Shift indicator state
    ShiftIndicatorState shift_indicator_state_ = ShiftIndicatorState::Normal;
    float shift_indicator_intensity_ = 0.0f;
    float shift_indicator_timer_ = 0.0f;
    float redline_threshold_ = 0.9f;  // RPM % to show redline

    // Gear shift validation
    float gear_change_cooldown_ = 0.0f;
    const float GEAR_CHANGE_COOLDOWN = 0.2f;  // 200ms between shifts

    // Notification display
    float notification_time_ = 0.0f;
    float notification_duration_ = 3.0f;
    std::string notification_text_;
    bool notification_active_ = false;

    // Blinking effect for shift indicator
    float blink_timer_ = 0.0f;
    float blink_frequency_ = 2.0f;  // 2 Hz blink

    bool initialized_ = false;

    // Helper methods
    void UpdateHUD(float deltaTime);
    void UpdateSpeedometer();
    void UpdateGearIndicator();
    void UpdateLapDisplay();
    void UpdatePositionDisplay();
    void UpdateDamageIndicator();
    void UpdateInputBars();
    void UpdateShiftIndicator(float deltaTime);
    void UpdateGearChangeCooldown(float deltaTime);
    void UpdateNotifications(float deltaTime);

    ShiftIndicatorState CalculateShiftIndicatorState() const;
    bool CanShiftGear() const;
    void DisplayNotification(const std::string& text, float duration = 3.0f);
    std::string FormatTime(float seconds) const;
    std::string FormatSpeed(float speed) const;
};
