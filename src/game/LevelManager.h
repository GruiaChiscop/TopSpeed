#pragma once

#include "GameManager.h"
#include <string>
#include <vector>
#include <memory>

/**
 * @class LevelManager
 * @brief Manages level progression, loading, and configuration
 */
class LevelManager {
public:
    struct LevelConfig {
        int level_number = 1;
        std::string level_name = "Level 1";
        int enemy_count = 3;
        int obstacle_count = 5;
        int powerup_count = 2;
        float enemy_damage_multiplier = 1.0f;
        float enemy_speed_multiplier = 1.0f;
        float time_limit = 0.0f;  // 0 = unlimited
        uint32_t target_score = 0;  // 0 = no score goal
        uint32_t level_reward = 1000;
    };

    LevelManager(GameManager* game_manager);
    ~LevelManager() = default;

    // Level loading and progression
    bool LoadLevel(int level_number);
    void NextLevel();
    void PreviousLevel();
    void RestartLevel();

    // Level configuration
    const LevelConfig& GetCurrentLevelConfig() const { return current_level_config_; }
    void SetLevelConfig(const LevelConfig& config);

    // Level spawning
    void SpawnLevelEntities();
    void ClearLevelEntities();

    // Level state
    int GetCurrentLevel() const { return current_level_; }
    int GetMaxLevel() const { return max_level_; }
    bool IsLastLevel() const { return current_level_ == max_level_; }

    // Level info
    std::string GetLevelName() const { return current_level_config_.level_name; }
    std::string GetLevelDescription() const;

    // Difficulty scaling
    float GetScaleFactor() const;
    void UpdateDifficultyScaling();

    // Utility
    std::string GetDebugInfo() const;

private:
    GameManager* game_manager_ = nullptr;
    int current_level_ = 1;
    int max_level_ = 5;
    LevelConfig current_level_config_;

    // Level configurations storage
    std::vector<LevelConfig> level_configs_;

    // Helper methods
    void InitializeLevelConfigs();
    void GenerateRandomLayout();
    void SpawnEnemies();
    void SpawnObstacles();
    void SpawnPowerUps();
};
