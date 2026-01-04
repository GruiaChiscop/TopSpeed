#include "RacingInput.h"
#include "../game/Vehicle.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

RacingInput::RacingInput(Vehicle* vehicle, InputSystem* input_system)
    : vehicle_(vehicle), input_system_(input_system) {
    SetupDefaultMappings();
}

RacingInput::~RacingInput() {
    Shutdown();
}

bool RacingInput::Initialize() {
    if (!vehicle_ || !input_system_) {
        return false;
    }

    SetupDefaultMappings();
    current_throttle_ = 0.0f;
    current_brake_ = 0.0f;
    current_steering_ = 0.0f;
    handbrake_active_ = false;
    automatic_transmission_ = true;
    current_gear_ = 0;

    return true;
}

void RacingInput::Shutdown() {
    current_throttle_ = 0.0f;
    current_brake_ = 0.0f;
    current_steering_ = 0.0f;
}

void RacingInput::Update(float deltaTime) {
    if (!vehicle_ || !input_system_) return;

    UpdateThrottleInput(deltaTime);
    UpdateBrakeInput(deltaTime);
    UpdateSteeringInput(deltaTime);
    HandleTransmission();
    ApplyInputToVehicle();
}

void RacingInput::UpdateThrottleInput(float deltaTime) {
    if (config_.use_analog_throttle) {
        target_throttle_ = GetAnalogThrottle();
    } else {
        target_throttle_ = GetDigitalThrottle();
    }

    // Smooth throttle transitions
    current_throttle_ = SmoothInput(current_throttle_, target_throttle_,
                                    config_.throttle_smoothing, deltaTime);

    current_throttle_ = std::max(0.0f, std::min(1.0f, current_throttle_));
}

void RacingInput::UpdateBrakeInput(float deltaTime) {
    if (config_.use_analog_throttle) {
        target_brake_ = GetAnalogBrake();
    } else {
        target_brake_ = GetDigitalBrake();
    }

    // Smooth brake transitions
    current_brake_ = SmoothInput(current_brake_, target_brake_,
                                 config_.brake_smoothing, deltaTime);

    current_brake_ = std::max(0.0f, std::min(1.0f, current_brake_));
}

void RacingInput::UpdateSteeringInput(float deltaTime) {
    if (config_.use_analog_steering) {
        target_steering_ = GetAnalogSteering();
    } else {
        target_steering_ = GetDigitalSteering();
    }

    // Apply deadzone
    if (std::abs(target_steering_) < config_.steering_deadzone) {
        target_steering_ = 0.0f;
    }

    // Apply sensitivity multiplier
    target_steering_ *= config_.steering_sensitivity;

    // Clamp to valid range
    target_steering_ = std::max(-1.0f, std::min(1.0f, target_steering_));

    // Steering is more responsive, use direct input
    current_steering_ = target_steering_;
}

float RacingInput::GetAnalogThrottle() const {
    if (!input_system_) return 0.0f;
    return input_system_->GetThrottle();
}

float RacingInput::GetAnalogBrake() const {
    if (!input_system_) return 0.0f;
    return input_system_->GetBrake();
}

float RacingInput::GetAnalogSteering() const {
    if (!input_system_) return 0.0f;
    return input_system_->GetSteering();
}

float RacingInput::GetDigitalThrottle() const {
    if (!input_system_) return 0.0f;
    if (input_system_->IsKeyDown(InputSystem::KeyCode::W) ||
        input_system_->IsKeyDown(InputSystem::KeyCode::Up)) {
        return 1.0f;
    }
    return 0.0f;
}

float RacingInput::GetDigitalBrake() const {
    if (!input_system_) return 0.0f;
    if (input_system_->IsKeyDown(InputSystem::KeyCode::S) ||
        input_system_->IsKeyDown(InputSystem::KeyCode::Down)) {
        return 1.0f;
    }
    return 0.0f;
}

float RacingInput::GetDigitalSteering() const {
    if (!input_system_) return 0.0f;
    float steering = 0.0f;
    if (input_system_->IsKeyDown(InputSystem::KeyCode::A) ||
        input_system_->IsKeyDown(InputSystem::KeyCode::Left)) {
        steering -= 1.0f;
    }
    if (input_system_->IsKeyDown(InputSystem::KeyCode::D) ||
        input_system_->IsKeyDown(InputSystem::KeyCode::Right)) {
        steering += 1.0f;
    }
    return steering;
}

float RacingInput::SmoothInput(float current, float target, float smoothing, float deltaTime) {
    // Exponential smoothing: output = output + (input - output) * factor
    // smoothing range: 0 (no smoothing) to 1 (infinite smoothing)
    if (smoothing <= 0.0f) {
        return target;  // No smoothing
    }
    if (smoothing >= 1.0f) {
        return current;  // Full smoothing (no change)
    }

    // Use frame-rate independent smoothing
    float factor = 1.0f - std::pow(1.0f - smoothing, deltaTime * 60.0f);
    return current + (target - current) * factor;
}

void RacingInput::ApplyInputToVehicle() {
    if (!vehicle_) return;

    // Apply throttle and brake
    if (current_throttle_ > 0.0f && current_brake_ == 0.0f) {
        vehicle_->Accelerate(current_throttle_, 0.016f);  // Assume ~60 FPS
    } else if (current_brake_ > 0.0f) {
        vehicle_->Brake(current_brake_, 0.016f);
    }

    // Apply steering
    if (current_steering_ != 0.0f) {
        vehicle_->Steer(current_steering_, 0.016f);
    }

    // Handle handbrake
    handbrake_active_ = input_system_ && input_system_->IsHandbrakePressed();
    if (handbrake_active_) {
        vehicle_->Brake(1.0f, 0.016f);
    }
}

void RacingInput::HandleTransmission() {
    if (!vehicle_ || automatic_transmission_) {
        return;  // Let vehicle handle automatic transmission
    }

    // Handle manual gear changes
    if (input_system_) {
        if (input_system_->IsShiftUpPressed()) {
            ShiftUp();
        }
        if (input_system_->IsShiftDownPressed()) {
            ShiftDown();
        }
    }
}

void RacingInput::ShiftUp() {
    if (vehicle_ && gear_change_cooldown_ <= 0.0f) {
        current_gear_++;
        vehicle_->ShiftUp();
        gear_change_cooldown_ = 0.2f;  // 200ms cooldown
    }
}

void RacingInput::ShiftDown() {
    if (vehicle_ && gear_change_cooldown_ <= 0.0f) {
        current_gear_--;
        if (current_gear_ < 1) {
            current_gear_ = 1;
        }
        vehicle_->ShiftDown();
        gear_change_cooldown_ = 0.2f;  // 200ms cooldown
    }
}

void RacingInput::MapAccelerateTo(InputSystem::KeyCode key) {
    if (input_system_) {
        input_system_->MapActionToKey(InputSystem::InputAction::Accelerate, key);
    }
}

void RacingInput::MapBrakeTo(InputSystem::KeyCode key) {
    if (input_system_) {
        input_system_->MapActionToKey(InputSystem::InputAction::Brake, key);
    }
}

void RacingInput::MapSteerLeftTo(InputSystem::KeyCode key) {
    if (input_system_) {
        input_system_->MapActionToKey(InputSystem::InputAction::SteerLeft, key);
    }
}

void RacingInput::MapSteerRightTo(InputSystem::KeyCode key) {
    if (input_system_) {
        input_system_->MapActionToKey(InputSystem::InputAction::SteerRight, key);
    }
}

void RacingInput::MapHandbrakeTo(InputSystem::KeyCode key) {
    if (input_system_) {
        input_system_->MapActionToKey(InputSystem::InputAction::Handbrake, key);
    }
}

void RacingInput::MapShiftUpTo(InputSystem::KeyCode key) {
    if (input_system_) {
        input_system_->MapActionToKey(InputSystem::InputAction::ShiftUp, key);
    }
}

void RacingInput::MapShiftDownTo(InputSystem::KeyCode key) {
    if (input_system_) {
        input_system_->MapActionToKey(InputSystem::InputAction::ShiftDown, key);
    }
}

void RacingInput::ResetMappings() {
    if (input_system_) {
        input_system_->ResetInputMapping();
    }
    SetupDefaultMappings();
}

void RacingInput::SetupDefaultMappings() {
    // Default mappings are set up in InputSystem
    // This method can add racing-specific overrides if needed
}

std::string RacingInput::GetDebugInfo() const {
    std::stringstream ss;
    ss << "RacingInput Debug Info\n";
    ss << "- Throttle: " << std::fixed << std::setprecision(2) << current_throttle_ << "\n";
    ss << "- Brake: " << current_brake_ << "\n";
    ss << "- Steering: " << current_steering_ << "\n";
    ss << "- Handbrake: " << (handbrake_active_ ? "Active" : "Inactive") << "\n";
    ss << "- Transmission: " << (automatic_transmission_ ? "Automatic" : "Manual") << "\n";
    ss << "- Current Gear: " << current_gear_ << "\n";
    ss << "- Steering Sensitivity: " << config_.steering_sensitivity << "\n";
    ss << "- Throttle Smoothing: " << config_.throttle_smoothing << "\n";
    ss << "- Brake Smoothing: " << config_.brake_smoothing << "\n";
    ss << "- Steering Deadzone: " << config_.steering_deadzone << "\n";
    ss << "- Use Analog Steering: " << (config_.use_analog_steering ? "Yes" : "No") << "\n";
    ss << "- Use Analog Throttle: " << (config_.use_analog_throttle ? "Yes" : "No") << "\n";
    return ss.str();
}
