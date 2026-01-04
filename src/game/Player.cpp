#include "Player.h"
#include <sstream>
#include <iomanip>

Player::Player(const std::string& name)
    : GameObject(name, Type::Player, glm::vec3(0.0f, 1.0f, 0.0f)) {
    health_ = 100.0f;
    mass_ = 1.0f;
    max_speed_ = 30.0f;
    collision_bounds_ = glm::vec3(1.0f, 2.0f, 1.0f);  // Player-sized collision box
    has_collision_ = true;
}

void Player::Initialize() {
    GameObject::Initialize();
    spawn_point_ = position_;
    score_ = 0;
    lives_ = 3;
    level_ = 1;
    health_ = 100.0f;
}

void Player::Update(float deltaTime) {
    if (!active_) return;

    // Update power-ups
    UpdateShield(deltaTime);
    UpdateSpeedBoost(deltaTime);
    UpdateDashCooldown(deltaTime);

    // Apply physics
    ApplyMovementPhysics(deltaTime);

    // Update position based on velocity
    ApplyVelocity(deltaTime);

    // Keep player in bounds (basic world bounds)
    if (position_.y < -10.0f) {
        LoseLife();
        Respawn(spawn_point_);
    }
}

void Player::Render() {
    // Rendering is handled by the game engine
    // This is just a marker for where render code would go
}

void Player::Shutdown() {
    GameObject::Shutdown();
}

void Player::MoveForward(float deltaTime) {
    glm::vec3 direction = glm::normalize(rotation_);
    velocity_.x += direction.x * acceleration_rate_ * deltaTime;
    velocity_.z += direction.z * acceleration_rate_ * deltaTime;
}

void Player::MoveBackward(float deltaTime) {
    glm::vec3 direction = glm::normalize(rotation_);
    velocity_.x -= direction.x * acceleration_rate_ * deltaTime;
    velocity_.z -= direction.z * acceleration_rate_ * deltaTime;
}

void Player::MoveLeft(float deltaTime) {
    rotation_.y += turn_speed_ * deltaTime;
}

void Player::MoveRight(float deltaTime) {
    rotation_.y -= turn_speed_ * deltaTime;
}

void Player::Jump(float force) {
    if (!is_jumping_ && current_ground_ != nullptr) {
        is_jumping_ = true;
        velocity_.y += force > 0.0f ? force : jump_force_;
    }
}

void Player::Dash(const glm::vec3& direction, float force) {
    if (CanDash()) {
        is_dashing_ = true;
        dash_duration_remaining_ = dash_duration_;
        dash_cooldown_ = dash_cooldown_max_;
        
        glm::vec3 dash_dir = glm::normalize(direction);
        velocity_ = dash_dir * (force > 0.0f ? force : dash_force_);
        
        AddScore(50);  // Dash bonus points
    }
}

void Player::HandleInput(const glm::vec3& input_direction) {
    if (glm::length(input_direction) > 0.1f) {
        glm::vec3 normalized = glm::normalize(input_direction);
        
        if (normalized.x > 0.1f) {
            MoveRight(0.016f);  // Approximate 60fps deltaTime
        } else if (normalized.x < -0.1f) {
            MoveLeft(0.016f);
        }
        
        if (normalized.z > 0.1f) {
            MoveForward(0.016f);
        } else if (normalized.z < -0.1f) {
            MoveBackward(0.016f);
        }
    }
}

void Player::HandleJump() {
    Jump();
}

void Player::HandleDash(const glm::vec3& dash_direction) {
    Dash(dash_direction);
}

void Player::ActivateShield(float duration) {
    shield_active_ = true;
    shield_duration_ = duration > 0.0f ? duration : shield_duration_max_;
}

void Player::DeactivateShield() {
    shield_active_ = false;
    shield_duration_ = 0.0f;
}

void Player::ActivateSpeedBoost(float multiplier, float duration) {
    speed_boost_active_ = true;
    speed_boost_multiplier_ = multiplier;
    speed_boost_duration_ = duration > 0.0f ? duration : speed_boost_duration_max_;
    max_speed_ *= multiplier;
}

void Player::DeactivateSpeedBoost() {
    if (speed_boost_active_ && speed_boost_multiplier_ > 0.0f) {
        max_speed_ /= speed_boost_multiplier_;
    }
    speed_boost_active_ = false;
    speed_boost_duration_ = 0.0f;
}

void Player::TakeDamage(float amount) {
    if (shield_active_) {
        DeactivateShield();
        return;  // Shield absorbs the damage
    }
    
    health_ -= amount;
    if (health_ <= 0.0f) {
        health_ = 0.0f;
        LoseLife();
        if (!IsGameOver()) {
            Respawn(spawn_point_);
        }
    }
}

void Player::Heal(float amount) {
    health_ = glm::min(health_ + amount, 100.0f);
}

void Player::Respawn(const glm::vec3& spawn_point) {
    position_ = spawn_point;
    velocity_ = glm::vec3(0.0f);
    health_ = 100.0f;
    is_jumping_ = false;
    is_dashing_ = false;
    shield_active_ = false;
    speed_boost_active_ = false;
}

void Player::OnCollisionEnter(GameObject& other) {
    // Handle collision logic
    switch (other.GetType()) {
        case Type::Enemy:
            TakeDamage(10.0f);
            break;
        case Type::PowerUp:
            // Power-up will handle the interaction
            break;
        case Type::Obstacle:
            TakeDamage(5.0f);
            break;
        default:
            break;
    }
}

void Player::OnCollisionStay(GameObject& other) {
    // Ground detection for jumping
    if (other.GetType() == Type::Obstacle) {
        current_ground_ = &other;
    }
}

void Player::OnCollisionExit(GameObject& other) {
    if (current_ground_ == &other) {
        current_ground_ = nullptr;
        is_jumping_ = true;
    }
}

void Player::UpdateDashCooldown(float deltaTime) {
    if (dash_cooldown_ > 0.0f) {
        dash_cooldown_ -= deltaTime;
    }
    
    if (is_dashing_) {
        dash_duration_remaining_ -= deltaTime;
        if (dash_duration_remaining_ <= 0.0f) {
            is_dashing_ = false;
        }
    }
}

void Player::UpdateShield(float deltaTime) {
    if (shield_active_) {
        shield_duration_ -= deltaTime;
        if (shield_duration_ <= 0.0f) {
            DeactivateShield();
        }
    }
}

void Player::UpdateSpeedBoost(float deltaTime) {
    if (speed_boost_active_) {
        speed_boost_duration_ -= deltaTime;
        if (speed_boost_duration_ <= 0.0f) {
            DeactivateSpeedBoost();
        }
    }
}

void Player::ApplyMovementPhysics(float deltaTime) {
    // Apply friction
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
    if (!current_ground_) {
        velocity_.y -= 9.81f * deltaTime;  // Gravity
    } else {
        velocity_.y = 0.0f;
        is_jumping_ = false;
    }
}

void Player::UpdateGroundContact() {
    // This would be called after collision checks
    // to determine if player is on ground
}

std::string Player::GetDebugInfo() const {
    std::stringstream ss;
    ss << "Player Debug Info\n";
    ss << "- Position: (" << std::fixed << std::setprecision(2)
       << position_.x << ", " << position_.y << ", " << position_.z << ")\n";
    ss << "- Velocity: (" << velocity_.x << ", " << velocity_.y << ", " << velocity_.z << ")\n";
    ss << "- Health: " << health_ << "%\n";
    ss << "- Score: " << score_ << "\n";
    ss << "- Lives: " << lives_ << "\n";
    ss << "- Level: " << level_ << "\n";
    ss << "- Speed: " << GetSpeed() << "/" << max_speed_ << "\n";
    ss << "- Shield: " << (shield_active_ ? "Active" : "Inactive") << "\n";
    ss << "- Speed Boost: " << (speed_boost_active_ ? "Active" : "Inactive") << "\n";
    ss << "- Dashing: " << (is_dashing_ ? "Yes" : "No") << "\n";
    ss << "- Jumping: " << (is_jumping_ ? "Yes" : "No") << "\n";
    ss << "- Game Over: " << (IsGameOver() ? "Yes" : "No");
    return ss.str();
}
