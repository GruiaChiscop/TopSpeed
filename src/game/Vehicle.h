#pragma once

#include "GameObject.h"
#include <glm/glm.hpp>

/**
 * @class Vehicle
 * @brief Base class for all racing vehicles (player car, opponents)
 * 
 * Extends GameObject with racing-specific physics and state management.
 * Represents a car on the track with speed, acceleration, steering, and crash mechanics.
 */
class Vehicle : public GameObject {
public:
    enum class VehicleState {
        Stopped,       // Not moving
        Starting,      // Engine starting
        Running,       // Normal operation
        Accelerating,  // Full throttle
        Braking,       // Slowing down
        Slipping,      // Loss of traction
        Crashing,      // Collision occurred
        Stopped_Crash  // Stopped after crash
    };

    enum class TransmissionType {
        Automatic,
        Manual
    };

    Vehicle(const std::string& name = "Vehicle");
    virtual ~Vehicle();

    // Lifecycle
    void Initialize() override;
    void Update(float deltaTime) override;
    void Shutdown() override;

    // Vehicle state
    VehicleState GetVehicleState() const { return vehicle_state_; }
    void SetVehicleState(VehicleState state) { vehicle_state_ = state; }

    // Speed and acceleration
    float GetSpeed() const { return speed_; }
    void SetSpeed(float speed) { speed_ = speed; }
    void Accelerate(float throttle, float deltaTime);
    void Brake(float brakeForce, float deltaTime);
    void Coast(float deltaTime);

    // Steering
    float GetSteeringAngle() const { return steering_angle_; }
    void SetSteeringAngle(float angle) { steering_angle_ = angle; }
    void Steer(float input, float deltaTime);

    // Physics parameters
    float GetMaxSpeed() const { return max_speed_; }
    void SetMaxSpeed(float speed) { max_speed_ = speed; }
    float GetAcceleration() const { return acceleration_; }
    void SetAcceleration(float accel) { acceleration_ = accel; }
    float GetDeceleration() const { return deceleration_; }
    void SetDeceleration(float decel) { deceleration_ = decel; }
    float GetFriction() const { return friction_; }
    void SetFriction(float friction) { friction_ = friction; }

    // Transmission
    TransmissionType GetTransmission() const { return transmission_; }
    void SetTransmission(TransmissionType trans) { transmission_ = trans; }
    int GetGear() const { return current_gear_; }
    void SetGear(int gear) { current_gear_ = gear; }
    void ShiftUp();
    void ShiftDown();
    int GetMaxGears() const { return max_gears_; }

    // Crash mechanics
    void Crash(const glm::vec3& impact_velocity = glm::vec3(0.0f));
    void MiniCrash(int damage = 10);
    bool HasCrashed() const { return has_crashed_; }
    void ResetCrashState() { has_crashed_ = false; crash_duration_ = 0.0f; }
    float GetCrashDuration() const { return crash_duration_; }

    // Lane position (for track navigation)
    float GetLanePosition() const { return lane_position_; }  // -1.0 to 1.0
    void SetLanePosition(float pos) { lane_position_ = glm::clamp(pos, -1.0f, 1.0f); }
    void ChangeLane(float direction, float deltaTime);

    // Distance traveled
    float GetDistanceTraveled() const { return distance_traveled_; }
    void SetDistanceTraveled(float dist) { distance_traveled_ = dist; }

    // Engine state
    bool IsEngineRunning() const { return engine_running_; }
    void StartEngine() { engine_running_ = true; }
    void StopEngine() { engine_running_ = false; speed_ = 0.0f; }

    // Damage and health
    int GetHealth() const { return health_; }
    void SetHealth(int hp) { health_ = glm::clamp(hp, 0, 100); }
    void TakeDamage(int damage);
    bool IsDestroyed() const { return health_ <= 0; }

    // Debug info
    std::string GetVehicleDebugInfo() const;

protected:
    VehicleState vehicle_state_ = VehicleState::Stopped;
    TransmissionType transmission_ = TransmissionType::Automatic;

    // Physics
    float speed_ = 0.0f;
    float max_speed_ = 100.0f;
    float acceleration_ = 30.0f;  // units per second^2
    float deceleration_ = 40.0f;  // units per second^2
    float friction_ = 0.95f;       // friction coefficient (0-1)
    float steering_angle_ = 0.0f;
    float max_steering_angle_ = 45.0f;  // degrees
    float steering_responsiveness_ = 5.0f;
    float lane_position_ = 0.0f;   // -1.0 (left) to 1.0 (right)
    float lane_change_speed_ = 2.0f;  // lane changes per second

    // Transmission
    int current_gear_ = 0;
    int max_gears_ = 5;
    float gear_ratios_[6] = {0.0f, 1.0f, 0.8f, 0.6f, 0.4f, 0.2f};  // simplified

    // Crash state
    bool has_crashed_ = false;
    float crash_duration_ = 0.0f;
    float max_crash_duration_ = 2.0f;  // seconds to recover
    glm::vec3 crash_impact_velocity_ = glm::vec3(0.0f);

    // Distance tracking
    float distance_traveled_ = 0.0f;

    // Engine
    bool engine_running_ = false;

    // Health
    int health_ = 100;

    // Helper methods
    void UpdatePhysics(float deltaTime);
    void UpdateCrashState(float deltaTime);
    void ApplyFriction(float deltaTime);
};
