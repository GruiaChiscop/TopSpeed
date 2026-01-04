#pragma once

#include "GameObject.h"

/**
 * @class PowerUp
 * @brief Collectible power-up that enhances player abilities
 */
class PowerUp : public GameObject {
public:
    enum class PowerUpType {
        HealthRestore,    // Restore player health
        Shield,           // Temporary invulnerability
        SpeedBoost,       // Temporary speed increase
        ScoreMultiplier,  // Temporary score boost
        ExtraLife,        // Gain extra life
        DoubleDamage      // Deal 2x damage temporarily
    };

    PowerUp(const std::string& name = "PowerUp", PowerUpType type = PowerUpType::HealthRestore);
    ~PowerUp() override = default;

    void Initialize() override;
    void Update(float deltaTime) override;
    void Render() override;
    void Shutdown() override;

    // Type and properties
    PowerUpType GetPowerUpType() const { return powerup_type_; }
    void SetPowerUpType(PowerUpType type) { powerup_type_ = type; }

    // Value (varies by type)
    float GetValue() const { return value_; }
    void SetValue(float value) { value_ = value; }

    // Duration (for temporary power-ups)
    float GetDuration() const { return duration_; }
    void SetDuration(float duration) { duration_ = duration; }

    // Lifetime (how long before despawn)
    float GetLifetime() const { return lifetime_; }
    void SetLifetime(float lifetime) { lifetime_ = lifetime; }
    float GetRemainingLifetime() const { return remaining_lifetime_; }

    // Spinning animation
    bool IsSpinning() const { return is_spinning_; }
    void SetSpinning(bool spinning) { is_spinning_ = spinning; }

    float GetSpinSpeed() const { return spin_speed_; }
    void SetSpinSpeed(float speed) { spin_speed_ = speed; }

    // Floating animation
    bool IsFloating() const { return is_floating_; }
    void SetFloating(bool floating) { is_floating_ = floating; }

    float GetFloatHeight() const { return float_height_; }
    void SetFloatHeight(float height) { float_height_ = height; }

    float GetFloatSpeed() const { return float_speed_; }
    void SetFloatSpeed(float speed) { float_speed_ = speed; }

    // Collision callbacks
    void OnCollisionEnter(GameObject& other) override;
    void OnCollisionStay(GameObject& other) override;
    void OnCollisionExit(GameObject& other) override;

    // Utility
    void Activate(GameObject& target);
    void Despawn();
    std::string GetDebugInfo() const;

private:
    PowerUpType powerup_type_ = PowerUpType::HealthRestore;

    // Effect parameters
    float value_ = 1.0f;           // Amount of healing, boost multiplier, etc.
    float duration_ = 5.0f;         // How long the effect lasts
    float lifetime_ = 30.0f;        // How long before despawn
    float remaining_lifetime_ = 30.0f;

    // Animations
    bool is_spinning_ = true;
    float spin_speed_ = 180.0f;     // degrees per second

    bool is_floating_ = true;
    float float_height_ = 0.5f;     // How far to float up and down
    float float_speed_ = 2.0f;      // Seconds per bob cycle
    float float_time_ = 0.0f;       // Time elapsed in bob cycle

    // Spawn location (for floating animation)
    glm::vec3 spawn_position_ = glm::vec3(0.0f);

    // Helper methods
    void UpdateAnimation(float deltaTime);
    void UpdateLifetime(float deltaTime);
    void ApplyEffect(GameObject& target);
};
