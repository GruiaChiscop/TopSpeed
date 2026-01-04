#include "Obstacle.h"
#include <glm/trigonometric.hpp>
#include <glm/matrix.hpp>
#include <sstream>
#include <iomanip>

Obstacle::Obstacle(const std::string& name, ObstacleType type)
    : GameObject(name, Type::Obstacle, glm::vec3(0.0f, 0.0f, 0.0f)),
      obstacle_type_(type) {
    has_collision_ = true;
    mass_ = 0.0f;  // Static by default
    collision_bounds_ = glm::vec3(2.0f, 2.0f, 2.0f);
    health_ = original_health_;
}

void Obstacle::Initialize() {
    GameObject::Initialize();
    health_ = original_health_;
    is_broken_ = false;
}

void Obstacle::Update(float deltaTime) {
    if (!active_ || is_broken_) return;

    UpdateDamageCooldown(deltaTime);

    if (is_moving_) {
        UpdateMovement(deltaTime);
    }

    if (is_rotating_) {
        UpdateRotation(deltaTime);
    }
}

void Obstacle::Render() {
    // Rendering handled by game engine
}

void Obstacle::Shutdown() {
    GameObject::Shutdown();
}

void Obstacle::Break() {
    if (is_breakable_) {
        is_broken_ = true;
        has_collision_ = false;
        visible_ = false;
    }
}

void Obstacle::Repair() {
    is_broken_ = false;
    has_collision_ = true;
    visible_ = true;
    health_ = original_health_;
}

void Obstacle::OnCollisionEnter(GameObject& other) {
    if (obstacle_type_ == ObstacleType::Pit) {
        // Pit is instant death - handled by player
    } else if (damage_ > 0.0f && damage_cooldown_remaining_ <= 0.0f) {
        // Deal damage on collision
        damage_cooldown_remaining_ = damage_cooldown_;
    }
}

void Obstacle::OnCollisionStay(GameObject& other) {
    // Continuous collision handling
}

void Obstacle::OnCollisionExit(GameObject& other) {
    // Collision ended
}

void Obstacle::UpdateMovement(float deltaTime) {
    glm::vec3 new_position = position_ + (movement_direction_ * movement_speed_ * deltaTime);

    // Check bounds
    if (new_position.x < min_bound_.x || new_position.x > max_bound_.x ||
        new_position.y < min_bound_.y || new_position.y > max_bound_.y ||
        new_position.z < min_bound_.z || new_position.z > max_bound_.z) {
        // Bounce back by reversing direction
        movement_direction_ = -movement_direction_;
    } else {
        position_ = new_position;
    }
}

void Obstacle::UpdateRotation(float deltaTime) {
    // Convert rotation speed from degrees to radians
    float rotation_rad = glm::radians(rotation_speed_ * deltaTime);

    // Update rotation around specified axis
    rotation_ += rotation_axis_ * rotation_rad;

    // Normalize rotation to prevent overflow
    if (glm::length(rotation_) > glm::two_pi<float>()) {
        rotation_ = glm::mod(rotation_, glm::two_pi<float>());
    }
}

void Obstacle::UpdateDamageCooldown(float deltaTime) {
    if (damage_cooldown_remaining_ > 0.0f) {
        damage_cooldown_remaining_ -= deltaTime;
    }
}

std::string Obstacle::GetDebugInfo() const {
    std::stringstream ss;
    ss << "Obstacle Debug Info\n";
    ss << "- Name: " << name_ << "\n";
    ss << "- Type: ";
    switch (obstacle_type_) {
        case ObstacleType::Wall:
            ss << "Wall";
            break;
        case ObstacleType::Spike:
            ss << "Spike";
            break;
        case ObstacleType::Pit:
            ss << "Pit";
            break;
        case ObstacleType::MovingBlock:
            ss << "MovingBlock";
            break;
        case ObstacleType::Rotating:
            ss << "Rotating";
            break;
        case ObstacleType::Breakable:
            ss << "Breakable";
            break;
    }
    ss << "\n";
    ss << "- Position: (" << std::fixed << std::setprecision(2)
       << position_.x << ", " << position_.y << ", " << position_.z << ")\n";
    ss << "- Health: " << health_ << "/" << original_health_ << "\n";
    ss << "- Damage: " << damage_ << "\n";
    ss << "- Broken: " << (is_broken_ ? "Yes" : "No") << "\n";
    ss << "- Moving: " << (is_moving_ ? "Yes" : "No");
    if (is_moving_) {
        ss << " (Speed: " << movement_speed_ << ")";
    }
    ss << "\n";
    ss << "- Rotating: " << (is_rotating_ ? "Yes" : "No");
    if (is_rotating_) {
        ss << " (Speed: " << rotation_speed_ << "°/s)";
    }
    return ss.str();
}
