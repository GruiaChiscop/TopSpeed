#include "Enemy.h"
#include <glm/geometric.hpp>
#include <sstream>
#include <iomanip>
#include <cmath>

Enemy::Enemy(const std::string& name)
    : GameObject(name, Type::Enemy, glm::vec3(0.0f, 1.0f, 0.0f)) {
    health_ = 50.0f;
    mass_ = 1.0f;
    max_speed_ = 15.0f;
    collision_bounds_ = glm::vec3(1.0f, 1.5f, 1.0f);
    has_collision_ = true;
}

void Enemy::Initialize() {
    GameObject::Initialize();
    ai_state_ = AIState::Idle;
    current_patrol_index_ = 0;
}

void Enemy::Update(float deltaTime) {
    if (!active_) return;

    // Update AI behavior
    UpdateAI(deltaTime);

    // Apply physics
    velocity_.x *= (1.0f - friction_);
    velocity_.z *= (1.0f - friction_);

    // Apply max speed limit
    float horizontal_speed = glm::length(glm::vec2(velocity_.x, velocity_.z));
    if (horizontal_speed > max_speed_) {
        float scale = max_speed_ / horizontal_speed;
        velocity_.x *= scale;
        velocity_.z *= scale;
    }

    // Apply gravity
    velocity_.y -= 9.81f * deltaTime;

    // Update position
    ApplyVelocity(deltaTime);

    // Despawn if fallen off world
    if (position_.y < -10.0f) {
        SetActive(false);
    }
}

void Enemy::Render() {
    // Rendering handled by game engine
}

void Enemy::Shutdown() {
    GameObject::Shutdown();
}

void Enemy::Chase(const glm::vec3& target) {
    chase_target_ = target;
    ai_state_ = AIState::Chasing;
}

void Enemy::Attack(GameObject& target) {
    ai_state_ = AIState::Attacking;
    PerformAttack(target);
}

void Enemy::Flee(const glm::vec3& away_from) {
    glm::vec3 flee_direction = glm::normalize(position_ - away_from);
    flee_target_ = position_ + (flee_direction * flee_distance_);
    ai_state_ = AIState::Fleeing;
}

void Enemy::Stun(float duration) {
    ai_state_ = AIState::Stunned;
    stunned_duration_ = duration;
    velocity_ = glm::vec3(0.0f);
}

void Enemy::OnCollisionEnter(GameObject& other) {
    if (other.GetType() == Type::Player) {
        // Try to attack or start chasing
        if (GetDistanceTo(other) < attack_range_) {
            Attack(other);
        }
    }
}

void Enemy::OnCollisionStay(GameObject& other) {
    // Maintain collision state
}

void Enemy::OnCollisionExit(GameObject& other) {
    // Collision ended
}

void Enemy::UpdateAI(float deltaTime) {
    switch (ai_state_) {
        case AIState::Idle:
            // Do nothing
            break;
        case AIState::Patrolling:
            UpdatePatrol(deltaTime);
            break;
        case AIState::Chasing:
            UpdateChase(deltaTime);
            break;
        case AIState::Attacking:
            // Attack cooldown handled separately
            break;
        case AIState::Fleeing:
            UpdateFlee(deltaTime);
            break;
        case AIState::Stunned:
            UpdateStunned(deltaTime);
            break;
    }

    // Update attack cooldown
    if (attack_cooldown_remaining_ > 0.0f) {
        attack_cooldown_remaining_ -= deltaTime;
    }
}

void Enemy::UpdatePatrol(float deltaTime) {
    if (patrol_points_.empty()) {
        ai_state_ = AIState::Idle;
        return;
    }

    glm::vec3 target = patrol_points_[current_patrol_index_];
    glm::vec3 direction = glm::normalize(target - position_);
    float distance = glm::distance(position_, target);

    if (distance < 1.0f) {
        // Reached patrol point, go to next
        current_patrol_index_ = (current_patrol_index_ + 1) % patrol_points_.size();
    } else {
        // Move towards patrol point
        velocity_.x = direction.x * patrol_speed_;
        velocity_.z = direction.z * patrol_speed_;
    }
}

void Enemy::UpdateChase(float deltaTime) {
    glm::vec3 direction = glm::normalize(chase_target_ - position_);
    float distance = glm::distance(position_, chase_target_);

    if (distance < 0.5f) {
        // Reached target, return to patrol
        ai_state_ = AIState::Patrolling;
    } else {
        // Chase target
        velocity_.x = direction.x * chase_speed_;
        velocity_.z = direction.z * chase_speed_;
    }
}

void Enemy::UpdateFlee(float deltaTime) {
    glm::vec3 direction = glm::normalize(flee_target_ - position_);
    float distance = glm::distance(position_, flee_target_);

    if (distance < 1.0f) {
        // Reached safe distance, return to patrol
        ai_state_ = AIState::Patrolling;
    } else {
        // Flee
        velocity_.x = direction.x * flee_speed_;
        velocity_.z = direction.z * flee_speed_;
    }
}

void Enemy::UpdateStunned(float deltaTime) {
    stunned_duration_ -= deltaTime;
    if (stunned_duration_ <= 0.0f) {
        // Stun ended, return to patrol
        ai_state_ = AIState::Patrolling;
        stunned_duration_ = 0.0f;
    }
}

void Enemy::PerformAttack(GameObject& target) {
    if (attack_cooldown_remaining_ <= 0.0f) {
        // Actually perform attack
        // Target will handle taking damage
        attack_cooldown_remaining_ = attack_cooldown_;
    }
}

glm::vec3 Enemy::GetNextPatrolPoint() const {
    if (patrol_points_.empty()) {
        return position_;
    }
    return patrol_points_[(current_patrol_index_ + 1) % patrol_points_.size()];
}

std::string Enemy::GetDebugInfo() const {
    std::stringstream ss;
    ss << "Enemy Debug Info\n";
    ss << "- Name: " << name_ << "\n";
    ss << "- Position: (" << std::fixed << std::setprecision(2)
       << position_.x << ", " << position_.y << ", " << position_.z << ")\n";
    ss << "- Velocity: (" << velocity_.x << ", " << velocity_.y << ", " << velocity_.z << ")\n";
    ss << "- Health: " << health_ << "/50\n";
    ss << "- AI State: ";
    switch (ai_state_) {
        case AIState::Idle:
            ss << "Idle";
            break;
        case AIState::Patrolling:
            ss << "Patrolling (" << current_patrol_index_ << "/" << patrol_points_.size() << ")";
            break;
        case AIState::Chasing:
            ss << "Chasing";
            break;
        case AIState::Attacking:
            ss << "Attacking";
            break;
        case AIState::Fleeing:
            ss << "Fleeing";
            break;
        case AIState::Stunned:
            ss << "Stunned (" << stunned_duration_ << "s remaining)";
            break;
    }
    ss << "\n";
    ss << "- Speed: " << GetSpeed() << "/" << max_speed_ << "\n";
    ss << "- Attack Cooldown: " << attack_cooldown_remaining_ << "s\n";
    ss << "- Reward: " << reward_points_ << " points";
    return ss.str();
}
