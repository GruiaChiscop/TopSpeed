#pragma once

#include "GameObject.h"
#include <cstdint>

/**
 * @class Player
 * @brief Player-controlled game object with score, lives, and input handling
 */
class Player : public GameObject {
public:
    Player(const std::string& name = "Player");
    ~Player() override = default;

    void Initialize() override;
    void Update(float deltaTime) override;
    void Render() override;
    void Shutdown() override;

    // Score management
    uint32_t GetScore() const { return score_; }
    void AddScore(uint32_t points) { score_ += points; }
    void SetScore(uint32_t score) { score_ = score; }
    void ResetScore() { score_ = 0; }

    // Lives management
    int GetLives() const { return lives_; }
    void SetLives(int lives) { lives_ = lives; }
    void LoseLife() { if (lives_ > 0) lives_--; }
    void GainLife() { lives_++; }
    bool IsGameOver() const { return lives_ <= 0; }

    // Level management
    int GetLevel() const { return level_; }
    void SetLevel(int level) { level_ = level; }
    void NextLevel() { level_++; ResetScore(); }

    // Movement
    void MoveForward(float deltaTime);
    void MoveBackward(float deltaTime);
    void MoveLeft(float deltaTime);
    void MoveRight(float deltaTime);
    void Jump(float force);
    void Dash(const glm::vec3& direction, float force);

    // State queries
    bool IsMoving() const { return GetSpeed() > 0.1f; }
    bool IsJumping() const { return is_jumping_; }
    bool IsDashing() const { return is_dashing_; }
    float GetDashCooldown() const { return dash_cooldown_; }
    bool CanDash() const { return dash_cooldown_ <= 0.0f; }

    // Input
    void HandleInput(const glm::vec3& input_direction);
    void HandleJump();
    void HandleDash(const glm::vec3& dash_direction);

    // Power-ups
    void ActivateShield(float duration);
    void DeactivateShield();
    bool HasShield() const { return shield_active_; }
    float GetShieldDuration() const { return shield_duration_; }

    void ActivateSpeedBoost(float multiplier, float duration);
    void DeactivateSpeedBoost();
    bool HasSpeedBoost() const { return speed_boost_active_; }
    float GetSpeedBoostMultiplier() const { return speed_boost_multiplier_; }

    // Collision callbacks
    void OnCollisionEnter(GameObject& other) override;
    void OnCollisionStay(GameObject& other) override;
    void OnCollisionExit(GameObject& other) override;

    // Damage and health
    void TakeDamage(float amount);
    void Heal(float amount);
    void Respawn(const glm::vec3& spawn_point);

    // Getters for debugging
    std::string GetDebugInfo() const;

private:
    // Game state
    uint32_t score_ = 0;
    int lives_ = 3;
    int level_ = 1;

    // Movement
    float acceleration_rate_ = 50.0f;
    float deceleration_rate_ = 30.0f;
    float turn_speed_ = 90.0f;  // degrees per second

    // Jumping
    bool is_jumping_ = false;
    float jump_force_ = 15.0f;
    glm::vec3 spawn_point_ = glm::vec3(0.0f, 1.0f, 0.0f);

    // Dashing
    bool is_dashing_ = false;
    float dash_cooldown_ = 0.0f;
    float dash_cooldown_max_ = 2.0f;  // seconds
    float dash_duration_ = 0.3f;
    float dash_duration_remaining_ = 0.0f;
    float dash_force_ = 50.0f;

    // Power-ups
    bool shield_active_ = false;
    float shield_duration_ = 0.0f;
    float shield_duration_max_ = 5.0f;

    bool speed_boost_active_ = false;
    float speed_boost_multiplier_ = 1.5f;
    float speed_boost_duration_ = 0.0f;
    float speed_boost_duration_max_ = 5.0f;

    // Collision tracking
    GameObject* current_ground_ = nullptr;

    // Helper methods
    void UpdateDashCooldown(float deltaTime);
    void UpdateShield(float deltaTime);
    void UpdateSpeedBoost(float deltaTime);
    void ApplyMovementPhysics(float deltaTime);
    void UpdateGroundContact();
};
