#include "GameObject.h"
#include <glm/geometric.hpp>
#include <cmath>

GameObject::GameObject(const std::string& name, Type type, const glm::vec3& position)
    : name_(name),
      type_(type),
      position_(position),
      velocity_(0.0f),
      scale_(1.0f),
      rotation_(0.0f),
      acceleration_(0.0f),
      collision_bounds_(1.0f) {}

bool GameObject::IsCollidingWith(const GameObject& other) const {
    if (!has_collision_ || !other.has_collision_) {
        return false;
    }

    // AABB collision detection
    glm::vec3 min_a = position_ - (collision_bounds_ * scale_ * 0.5f);
    glm::vec3 max_a = position_ + (collision_bounds_ * scale_ * 0.5f);

    glm::vec3 min_b = other.position_ - (other.collision_bounds_ * other.scale_ * 0.5f);
    glm::vec3 max_b = other.position_ + (other.collision_bounds_ * other.scale_ * 0.5f);

    return (min_a.x <= max_b.x && max_a.x >= min_b.x) &&
           (min_a.y <= max_b.y && max_a.y >= min_b.y) &&
           (min_a.z <= max_b.z && max_a.z >= min_b.z);
}

bool GameObject::IsCollidingWithPoint(const glm::vec3& point) const {
    if (!has_collision_) {
        return false;
    }

    glm::vec3 min = position_ - (collision_bounds_ * scale_ * 0.5f);
    glm::vec3 max = position_ + (collision_bounds_ * scale_ * 0.5f);

    return (point.x >= min.x && point.x <= max.x) &&
           (point.y >= min.y && point.y <= max.y) &&
           (point.z >= min.z && point.z <= max.z);
}

glm::vec3 GameObject::GetCollisionNormal(const GameObject& other) const {
    // Simple collision normal based on centers
    glm::vec3 dir = other.position_ - position_;
    if (glm::length(dir) > 0.001f) {
        return glm::normalize(dir);
    }
    return glm::vec3(1.0f, 0.0f, 0.0f);
}

float GameObject::GetDistanceTo(const GameObject& other) const {
    return glm::distance(position_, other.position_);
}

float GameObject::GetDistanceTo(const glm::vec3& point) const {
    return glm::distance(position_, point);
}

glm::vec3 GameObject::GetDirectionTo(const GameObject& other) const {
    glm::vec3 dir = other.position_ - position_;
    float len = glm::length(dir);
    if (len > 0.001f) {
        return dir / len;
    }
    return glm::vec3(0.0f);
}

glm::vec3 GameObject::GetDirectionTo(const glm::vec3& point) const {
    glm::vec3 dir = point - position_;
    float len = glm::length(dir);
    if (len > 0.001f) {
        return dir / len;
    }
    return glm::vec3(0.0f);
}

void GameObject::ApplyForce(const glm::vec3& force) {
    if (mass_ > 0.0f) {
        acceleration_ += force / mass_;
    }
}

void GameObject::ApplyImpulse(const glm::vec3& impulse) {
    if (mass_ > 0.0f) {
        velocity_ += impulse / mass_;
    }
}
