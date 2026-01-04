#pragma once

#include "InputSystem.h"
#include <memory>

class Vehicle;  // Forward declaration

/**
 * @class RacingInput
 * @brief Handles racing-specific input and applies it to vehicle
 * 
 * Bridges InputSystem and Vehicle, converting player input into vehicle commands.
 * Handles throttle smoothing, steering responsiveness, and transmission control.
 */
class RacingInput {
public:
    struct RacingInputConfig {
        float steering_sensitivity = 1.5f;   // Multiplier for steering input
        float throttle_smoothing = 0.3f;     // Interpolation factor (0-1)
        float brake_smoothing = 0.2f;        // Interpolation factor (0-1)
        float steering_deadzone = 0.05f;     // Steering axis deadzone
        bool use_analog_steering = true;     // Use analog or digital steering
        bool use_analog_throttle = true;     // Use analog or digital throttle
    };

    RacingInput(Vehicle* vehicle, InputSystem* input_system);
    ~RacingInput();

    // Initialization
    bool Initialize();
    void Shutdown();

    // Update (call from game loop)
    void Update(float deltaTime);

    // Configuration
    void SetConfig(const RacingInputConfig& config) { config_ = config; }
    RacingInputConfig GetConfig() const { return config_; }

    // Input state queries
    float GetCurrentThrottle() const { return current_throttle_; }
    float GetCurrentBrake() const { return current_brake_; }
    float GetCurrentSteering() const { return current_steering_; }
    bool IsHandbrakeActive() const { return handbrake_active_; }

    // Manual transmission control
    void ShiftUp();
    void ShiftDown();
    void SetAutomatic(bool automatic) { automatic_transmission_ = automatic; }
    bool IsAutomatic() const { return automatic_transmission_; }

    // Input remapping for racing
    void MapAccelerateTo(InputSystem::KeyCode key);
    void MapBrakeTo(InputSystem::KeyCode key);
    void MapSteerLeftTo(InputSystem::KeyCode key);
    void MapSteerRightTo(InputSystem::KeyCode key);
    void MapHandbrakeTo(InputSystem::KeyCode key);
    void MapShiftUpTo(InputSystem::KeyCode key);
    void MapShiftDownTo(InputSystem::KeyCode key);
    void ResetMappings();

    // Debug
    std::string GetDebugInfo() const;

private:
    Vehicle* vehicle_;              // Reference to controlled vehicle
    InputSystem* input_system_;     // Reference to input system
    RacingInputConfig config_;

    // Smoothed input values
    float current_throttle_ = 0.0f;
    float current_brake_ = 0.0f;
    float current_steering_ = 0.0f;
    float target_throttle_ = 0.0f;
    float target_brake_ = 0.0f;
    float target_steering_ = 0.0f;

    // Transmission state
    bool automatic_transmission_ = true;
    int current_gear_ = 0;
    float gear_change_cooldown_ = 0.0f;

    // State tracking
    bool handbrake_active_ = false;
    bool engine_started_ = false;
    float last_throttle_input_ = 0.0f;
    float last_brake_input_ = 0.0f;

    // Helper methods
    void UpdateThrottleInput(float deltaTime);
    void UpdateBrakeInput(float deltaTime);
    void UpdateSteeringInput(float deltaTime);
    void ApplyInputToVehicle();
    void HandleTransmission();
    float SmoothInput(float current, float target, float smoothing, float deltaTime);
    float GetAnalogThrottle() const;
    float GetAnalogBrake() const;
    float GetAnalogSteering() const;
    float GetDigitalThrottle() const;
    float GetDigitalBrake() const;
    float GetDigitalSteering() const;
    void SetupDefaultMappings();
};
