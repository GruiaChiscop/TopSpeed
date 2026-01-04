#pragma once

#include "GameObject.h"

/**
 * @class Enemy
 * @brief Enemy game object with AI behavior and attack patterns
 */
class Enemy : public GameObject {
public:
    enum class AIState {
        Idle,       // Waiting
        Patrolling, // Moving in pattern
        Chasing,    // Pursuing player
        Attacking,  // Attacking player
        Fleeing,    // Running away
        Stunned     // Disabled temporarily
    };

    Enemy(const std::string& name = "Enemy");
    ~Enemy() override = default;

    void Initialize() override;
    void Update(float deltaTime) override;
    void Render() override;
    void Shutdown() override;

    // AI State
    AIState GetAIState() const { return ai_state_; }
    void SetAIState(AIState state) { ai_state_ = state; }

    // Behavior
    void SetPatrolPoints(const std::vector<glm::vec3>& points) { patrol_points_ = points; }
    void Chase(const glm::vec3& target);
    void Attack(GameObject& target);
    void Flee(const glm::vec3& away_from);
    void Stun(float duration);

    // AI parameters
    float GetDetectionRange() const { return detection_range_; }
    void SetDetectionRange(float range) { detection_range_ = range; }

    float GetAttackRange() const { return attack_range_; }
    void SetAttackRange(float range) { attack_range_ = range; }

    float GetAttackDamage() const { return attack_damage_; }
    void SetAttackDamage(float damage) { attack_damage_ = damage; }

    float GetAttackCooldown() const { return attack_cooldown_; }
    float GetAttackCooldownRemaining() const { return attack_cooldown_remaining_; }

    // Rewards
    uint32_t GetRewardPoints() const { return reward_points_; }
    void SetRewardPoints(uint32_t points) { reward_points_ = points; }

    // Collision callbacks
    void OnCollisionEnter(GameObject& other) override;
    void OnCollisionStay(GameObject& other) override;
    void OnCollisionExit(GameObject& other) override;

    // Utility
    std::string GetDebugInfo() const;

private:
    // AI
    AIState ai_state_ = AIState::Idle;
    float stunned_duration_ = 0.0f;

    // Patrol
    std::vector<glm::vec3> patrol_points_;
    size_t current_patrol_index_ = 0;
    float patrol_speed_ = 5.0f;

    // Chase
    glm::vec3 chase_target_ = glm::vec3(0.0f);
    float chase_speed_ = 15.0f;
    float detection_range_ = 20.0f;  // How far can enemy see
    float detection_timer_ = 0.0f;
    float detection_check_interval_ = 0.5f;  // Check every 0.5 seconds

    // Attack
    float attack_range_ = 5.0f;
    float attack_damage_ = 10.0f;
    float attack_cooldown_ = 2.0f;  // Seconds between attacks
    float attack_cooldown_remaining_ = 0.0f;

    // Flee
    float flee_speed_ = 10.0f;
    float flee_distance_ = 30.0f;  // How far to flee
    glm::vec3 flee_target_ = glm::vec3(0.0f);

    // Rewards
    uint32_t reward_points_ = 100;

    // Helper methods
    void UpdateAI(float deltaTime);
    void UpdatePatrol(float deltaTime);
    void UpdateChase(float deltaTime);
    void UpdateFlee(float deltaTime);
    void UpdateStunned(float deltaTime);
    void PerformAttack(GameObject& target);
    glm::vec3 GetNextPatrolPoint() const;
};
