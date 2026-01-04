#include "Vehicle.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

Vehicle::Vehicle(const std::string& name)
    : GameObject(name) {
}

Vehicle::~Vehicle() {
    Shutdown();
}

void Vehicle::Initialize() {
    GameObject::Initialize();
    engine_running_ = false;
    speed_ = 0.0f;
    current_gear_ = 0;
    lane_position_ = 0.0f;
    steering_angle_ = 0.0f;
    vehicle_state_ = VehicleState::Stopped;
    health_ = 100;
    has_crashed_ = false;
    crash_duration_ = 0.0f;
}

void Vehicle::Update(float deltaTime) {
    GameObject::Update(deltaTime);

    if (!engine_running_) {
        speed_ = 0.0f;
        return;
    }

    UpdatePhysics(deltaTime);
    UpdateCrashState(deltaTime);

    // Update position based on speed and steering
    if (speed_ > 0.01f) {
        distance_traveled_ += speed_ * deltaTime;
    }
}

void Vehicle::Shutdown() {
    StopEngine();
    GameObject::Shutdown();
}

void Vehicle::Accelerate(float throttle, float deltaTime) {
    if (!engine_running_ || vehicle_state_ == VehicleState::Crashed) {
        return;
    }

    // Clamp throttle to 0-1
    throttle = glm::clamp(throttle, 0.0f, 1.0f);

    // Apply acceleration
    float accel = acceleration_ * throttle;
    speed_ += accel * deltaTime;

    // Cap speed
    speed_ = glm::clamp(speed_, 0.0f, max_speed_);

    // Update vehicle state
    if (throttle > 0.5f) {
        vehicle_state_ = VehicleState::Accelerating;
    } else if (speed_ > 0.01f) {
        vehicle_state_ = VehicleState::Running;
    }
}

void Vehicle::Brake(float brakeForce, float deltaTime) {
    if (!engine_running_) {
        return;
    }

    brakeForce = glm::clamp(brakeForce, 0.0f, 1.0f);

    // Apply braking
    float decel = deceleration_ * brakeForce;
    speed_ -= decel * deltaTime;

    // Prevent negative speed
    speed_ = glm::clamp(speed_, 0.0f, max_speed_);

    if (brakeForce > 0.1f) {
        vehicle_state_ = VehicleState::Braking;
    }
}

void Vehicle::Coast(float deltaTime) {
    if (!engine_running_ || speed_ <= 0.01f) {
        speed_ = 0.0f;
        return;
    }

    // Natural deceleration due to friction
    ApplyFriction(deltaTime);

    if (speed_ > 0.01f) {
        vehicle_state_ = VehicleState::Running;
    } else {
        speed_ = 0.0f;
        vehicle_state_ = VehicleState::Stopped;
    }
}

void Vehicle::Steer(float input, float deltaTime) {
    if (!engine_running_) {
        steering_angle_ = 0.0f;
        return;
    }

    // Clamp input to -1 to 1
    input = glm::clamp(input, -1.0f, 1.0f);

    // Target steering angle
    float target_angle = input * max_steering_angle_;

    // Smoothly interpolate steering
    float delta = target_angle - steering_angle_;
    float max_change = steering_responsiveness_ * deltaTime;
    steering_angle_ += glm::clamp(delta, -max_change, max_change);

    // Update lane position based on steering (simplified)
    lane_position_ += (input * 0.5f) * deltaTime;
    lane_position_ = glm::clamp(lane_position_, -1.0f, 1.0f);
}

void Vehicle::ShiftUp() {
    if (transmission_ == TransmissionType::Automatic) {
        return;  // Auto transmission handles gear changes
    }

    if (current_gear_ < max_gears_) {
        current_gear_++;
    }
}

void Vehicle::ShiftDown() {
    if (transmission_ == TransmissionType::Automatic) {
        return;  // Auto transmission handles gear changes
    }

    if (current_gear_ > 0) {
        current_gear_--;
    }
}

void Vehicle::Crash(const glm::vec3& impact_velocity) {
    if (vehicle_state_ == VehicleState::Crashed) {
        return;  // Already crashed
    }

    has_crashed_ = true;
    crash_duration_ = 0.0f;
    crash_impact_velocity_ = impact_velocity;
    vehicle_state_ = VehicleState::Crashing;

    // Reduce speed on crash
    speed_ *= 0.5f;

    // Take damage
    TakeDamage(25);
}

void Vehicle::MiniCrash(int damage) {
    TakeDamage(damage);
    speed_ *= 0.8f;  // Slight speed reduction

    // Temporary slipping effect
    if (vehicle_state_ != VehicleState::Crashing) {
        vehicle_state_ = VehicleState::Slipping;
    }
}

void Vehicle::ChangeLane(float direction, float deltaTime) {
    direction = glm::clamp(direction, -1.0f, 1.0f);
    lane_position_ += direction * lane_change_speed_ * deltaTime;
    lane_position_ = glm::clamp(lane_position_, -1.0f, 1.0f);
}

void Vehicle::TakeDamage(int damage) {
    health_ -= damage;
    health_ = glm::clamp(health_, 0, 100);

    if (health_ <= 0) {
        StopEngine();
        vehicle_state_ = VehicleState::Stopped_Crash;
    }
}

void Vehicle::UpdatePhysics(float deltaTime) {
    // Apply friction naturally
    ApplyFriction(deltaTime);
}

void Vehicle::UpdateCrashState(float deltaTime) {
    if (vehicle_state_ == VehicleState::Crashing || vehicle_state_ == VehicleState::Slipping) {
        crash_duration_ += deltaTime;

        if (crash_duration_ >= max_crash_duration_) {
            has_crashed_ = false;
            crash_duration_ = 0.0f;
            vehicle_state_ = VehicleState::Running;
        }
    }
}

void Vehicle::ApplyFriction(float deltaTime) {
    if (speed_ <= 0.01f) {
        speed_ = 0.0f;
        return;
    }

    // Apply friction/air resistance
    speed_ *= std::pow(friction_, deltaTime);

    if (speed_ < 0.01f) {
        speed_ = 0.0f;
    }
}

std::string Vehicle::GetVehicleDebugInfo() const {
    std::stringstream ss;
    ss << "Vehicle Debug Info - " << name_ << "\n";
    ss << "- State: ";
    switch (vehicle_state_) {
        case VehicleState::Stopped:
            ss << "Stopped";
            break;
        case VehicleState::Starting:
            ss << "Starting";
            break;
        case VehicleState::Running:
            ss << "Running";
            break;
        case VehicleState::Accelerating:
            ss << "Accelerating";
            break;
        case VehicleState::Braking:
            ss << "Braking";
            break;
        case VehicleState::Slipping:
            ss << "Slipping";
            break;
        case VehicleState::Crashing:
            ss << "Crashing";
            break;
        case VehicleState::Stopped_Crash:
            ss << "Stopped (Crashed)";
            break;
    }
    ss << "\n";
    ss << "- Speed: " << std::fixed << std::setprecision(2) << speed_ << " / " << max_speed_ << "\n";
    ss << "- Gear: " << current_gear_ << " / " << max_gears_ << "\n";
    ss << "- Lane Position: " << lane_position_ << "\n";
    ss << "- Steering Angle: " << steering_angle_ << "°\n";
    ss << "- Health: " << health_ << "%\n";
    ss << "- Distance: " << distance_traveled_ << "\n";
    ss << "- Engine Running: " << (engine_running_ ? "Yes" : "No") << "\n";
    ss << "- Crashed: " << (has_crashed_ ? "Yes" : "No");
    return ss.str();
}
