#pragma once

#include "GameObject.h"

/**
 * @class Obstacle
 * @brief Static or moving obstacle that blocks player movement and causes damage
 */
class Obstacle : public GameObject {
public:
    enum class ObstacleType {
        Wall,          // Solid wall, blocks movement
        Spike,         // Damage on contact
        Pit,           // Instant death
        MovingBlock,   // Moving obstacle
        Rotating,      // Rotating obstacle
        Breakable      // Can be destroyed
    };

    Obstacle(const std::string& name = "Obstacle", ObstacleType type = ObstacleType::Wall);
    ~Obstacle() override = default;

    void Initialize() override;
    void Update(float deltaTime) override;
    void Render() override;
    void Shutdown() override;

    // Type and properties
    ObstacleType GetObstacleType() const { return obstacle_type_; }
    void SetObstacleType(ObstacleType type) { obstacle_type_ = type; }

    // Damage
    float GetDamage() const { return damage_; }
    void SetDamage(float damage) { damage_ = damage; }

    // Movement
    bool IsMoving() const { return is_moving_; }
    void SetMoving(bool moving) { is_moving_ = moving; }

    const glm::vec3& GetMovementDirection() const { return movement_direction_; }
    void SetMovementDirection(const glm::vec3& dir) { movement_direction_ = glm::normalize(dir); }

    float GetMovementSpeed() const { return movement_speed_; }
    void SetMovementSpeed(float speed) { movement_speed_ = speed; }

    // Movement bounds (for moving obstacles)
    const glm::vec3& GetMinBound() const { return min_bound_; }
    void SetMinBound(const glm::vec3& bound) { min_bound_ = bound; }

    const glm::vec3& GetMaxBound() const { return max_bound_; }
    void SetMaxBound(const glm::vec3& bound) { max_bound_ = bound; }

    // Rotation (for rotating obstacles)
    bool IsRotating() const { return is_rotating_; }
    void SetRotating(bool rotating) { is_rotating_ = rotating; }

    float GetRotationSpeed() const { return rotation_speed_; }
    void SetRotationSpeed(float speed) { rotation_speed_ = speed; }

    const glm::vec3& GetRotationAxis() const { return rotation_axis_; }
    void SetRotationAxis(const glm::vec3& axis) { rotation_axis_ = glm::normalize(axis); }

    // Breakable
    bool IsBreakable() const { return is_breakable_; }
    void SetBreakable(bool breakable) { is_breakable_ = breakable; }

    bool IsBroken() const { return is_broken_; }
    void Break();
    void Repair();

    // Collision callbacks
    void OnCollisionEnter(GameObject& other) override;
    void OnCollisionStay(GameObject& other) override;
    void OnCollisionExit(GameObject& other) override;

    // Utility
    std::string GetDebugInfo() const;

private:
    ObstacleType obstacle_type_ = ObstacleType::Wall;

    // Damage
    float damage_ = 10.0f;
    float damage_cooldown_ = 0.5f;  // Cooldown between damage hits
    float damage_cooldown_remaining_ = 0.0f;

    // Movement
    bool is_moving_ = false;
    glm::vec3 movement_direction_ = glm::vec3(1.0f, 0.0f, 0.0f);
    float movement_speed_ = 5.0f;
    glm::vec3 min_bound_ = glm::vec3(-10.0f, -10.0f, -10.0f);
    glm::vec3 max_bound_ = glm::vec3(10.0f, 10.0f, 10.0f);

    // Rotation
    bool is_rotating_ = false;
    glm::vec3 rotation_axis_ = glm::vec3(0.0f, 1.0f, 0.0f);
    float rotation_speed_ = 90.0f;  // degrees per second

    // Breakable
    bool is_breakable_ = false;
    bool is_broken_ = false;
    float original_health_ = 30.0f;

    // Helper methods
    void UpdateMovement(float deltaTime);
    void UpdateRotation(float deltaTime);
    void UpdateDamageCooldown(float deltaTime);
};
