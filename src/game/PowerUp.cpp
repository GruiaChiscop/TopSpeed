#include "PowerUp.h"
#include "Player.h"
#include <glm/trigonometric.hpp>
#include <sstream>
#include <iomanip>
#include <cmath>

PowerUp::PowerUp(const std::string& name, PowerUpType type)
    : GameObject(name, Type::PowerUp, glm::vec3(0.0f, 1.0f, 0.0f)),
      powerup_type_(type) {
    has_collision_ = true;
    collision_bounds_ = glm::vec3(0.5f, 0.5f, 0.5f);
    mass_ = 0.0f;  // Static
    spawn_position_ = position_;
}

void PowerUp::Initialize() {
    GameObject::Initialize();
    spawn_position_ = position_;
    remaining_lifetime_ = lifetime_;
    float_time_ = 0.0f;
}

void PowerUp::Update(float deltaTime) {
    if (!active_) return;

    UpdateAnimation(deltaTime);
    UpdateLifetime(deltaTime);
}

void PowerUp::Render() {
    // Rendering handled by game engine
}

void PowerUp::Shutdown() {
    GameObject::Shutdown();
}

void PowerUp::OnCollisionEnter(GameObject& other) {
    if (other.GetType() == Type::Player) {
        Activate(other);
    }
}

void PowerUp::OnCollisionStay(GameObject& other) {
    // No action
}

void PowerUp::OnCollisionExit(GameObject& other) {
    // No action
}

void PowerUp::Activate(GameObject& target) {
    if (target.GetType() == Type::Player) {
        Player* player = dynamic_cast<Player*>(&target);
        if (player) {
            ApplyEffect(*player);
            Despawn();
        }
    }
}

void PowerUp::Despawn() {
    active_ = false;
    visible_ = false;
    has_collision_ = false;
}

void PowerUp::UpdateAnimation(float deltaTime) {
    if (is_spinning_) {
        float rotation_rad = glm::radians(spin_speed_ * deltaTime);
        rotation_.y += rotation_rad;
        if (rotation_.y > glm::two_pi<float>()) {
            rotation_.y -= glm::two_pi<float>();
        }
    }

    if (is_floating_) {
        float_time_ += deltaTime;
        float cycle = 2.0f / float_speed_;  // Full cycle time
        if (float_time_ > cycle) {
            float_time_ = 0.0f;
        }

        // Sine wave for smooth floating
        float offset = std::sin((float_time_ / cycle) * glm::two_pi<float>()) * float_height_;
        position_.y = spawn_position_.y + offset;
    }
}

void PowerUp::UpdateLifetime(float deltaTime) {
    remaining_lifetime_ -= deltaTime;
    if (remaining_lifetime_ <= 0.0f) {
        Despawn();
    }
}

void PowerUp::ApplyEffect(GameObject& target) {
    Player* player = dynamic_cast<Player*>(&target);
    if (!player) return;

    switch (powerup_type_) {
        case PowerUpType::HealthRestore:
            player->Heal(value_);
            break;

        case PowerUpType::Shield:
            player->ActivateShield(duration_);
            break;

        case PowerUpType::SpeedBoost:
            player->ActivateSpeedBoost(value_, duration_);
            break;

        case PowerUpType::ScoreMultiplier:
            // Score multiplier would be handled by game manager
            // For now, just give bonus points
            player->AddScore(static_cast<uint32_t>(value_ * 100));
            break;

        case PowerUpType::ExtraLife:
            player->GainLife();
            break;

        case PowerUpType::DoubleDamage:
            // Double damage would be a stat modifier
            // For now, give score bonus
            player->AddScore(500);
            break;
    }
}

std::string PowerUp::GetDebugInfo() const {
    std::stringstream ss;
    ss << "PowerUp Debug Info\n";
    ss << "- Name: " << name_ << "\n";
    ss << "- Type: ";
    switch (powerup_type_) {
        case PowerUpType::HealthRestore:
            ss << "HealthRestore";
            break;
        case PowerUpType::Shield:
            ss << "Shield";
            break;
        case PowerUpType::SpeedBoost:
            ss << "SpeedBoost";
            break;
        case PowerUpType::ScoreMultiplier:
            ss << "ScoreMultiplier";
            break;
        case PowerUpType::ExtraLife:
            ss << "ExtraLife";
            break;
        case PowerUpType::DoubleDamage:
            ss << "DoubleDamage";
            break;
    }
    ss << "\n";
    ss << "- Position: (" << std::fixed << std::setprecision(2)
       << position_.x << ", " << position_.y << ", " << position_.z << ")\n";
    ss << "- Value: " << value_ << "\n";
    ss << "- Duration: " << duration_ << "s\n";
    ss << "- Remaining Lifetime: " << remaining_lifetime_ << "s\n";
    ss << "- Spinning: " << (is_spinning_ ? "Yes" : "No") << "\n";
    ss << "- Floating: " << (is_floating_ ? "Yes" : "No");
    return ss.str();
}
