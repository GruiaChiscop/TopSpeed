#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

/**
 * @class ScoringSystem
 * @brief Manages all scoring logic, multipliers, and achievements
 */
class ScoringSystem {
public:
    enum class ScoreEvent {
        EnemyDefeat,        // Defeating an enemy
        ObstacleDestroy,    // Destroying breakable obstacle
        PowerUpCollect,     // Collecting power-up
        LevelComplete,      // Completing level
        NoHitBonus,         // Completing level without taking damage
        SpeedBonus,         // Time-based bonus
        ComboKill,          // Defeating multiple enemies quickly
        Perfect,            // Perfect level (all objectives)
        Custom              // Custom score event
    };

    ScoringSystem();
    ~ScoringSystem() = default;

    // Score getters/setters
    uint32_t GetTotalScore() const { return total_score_; }
    void SetTotalScore(uint32_t score) { total_score_ = score; }
    void ResetScore() { total_score_ = 0; }

    // Score events
    uint32_t AddScore(ScoreEvent event, uint32_t base_points = 0);
    uint32_t AddScore(uint32_t points);

    // Base point values
    uint32_t GetBasePoints(ScoreEvent event) const;
    void SetBasePoints(ScoreEvent event, uint32_t points);

    // Multipliers
    float GetScoreMultiplier() const { return score_multiplier_; }
    void SetScoreMultiplier(float multiplier) { score_multiplier_ = multiplier; }

    float GetComboMultiplier() const { return combo_multiplier_; }
    void SetComboMultiplier(float multiplier) { combo_multiplier_ = multiplier; }

    float GetDifficultyMultiplier() const { return difficulty_multiplier_; }
    void SetDifficultyMultiplier(float multiplier) { difficulty_multiplier_ = multiplier; }

    float GetTimeMultiplier() const { return time_multiplier_; }
    void SetTimeMultiplier(float multiplier) { time_multiplier_ = multiplier; }

    // Combo system
    int GetComboCount() const { return combo_count_; }
    void IncrementCombo();
    void ResetCombo();
    float GetComboDecayRate() const { return combo_decay_rate_; }
    void SetComboDecayRate(float rate) { combo_decay_rate_ = rate; }

    // Time-based bonuses
    uint32_t GetTimeBonus(float time_elapsed) const;
    float GetTimeBonusMultiplier(float time_elapsed) const;

    // Damage penalty
    float GetDamagePenalty() const { return damage_penalty_; }
    void SetDamagePenalty(float penalty) { damage_penalty_ = penalty; }

    // Streak tracking
    int GetKillStreak() const { return kill_streak_; }
    void IncrementKillStreak();
    void ResetKillStreak();

    // Accuracy tracking
    float GetAccuracy() const;
    void RecordHit() { hits_++; }
    void RecordMiss() { misses_++; }
    void ResetAccuracy() { hits_ = 0; misses_ = 0; }

    // Level statistics
    struct LevelStats {
        uint32_t level_score = 0;
        uint32_t enemies_defeated = 0;
        uint32_t damage_taken = 0;
        float time_taken = 0.0f;
        int bonuses_collected = 0;
        bool perfect = false;
    };

    LevelStats GetCurrentLevelStats() const { return current_level_stats_; }
    void ResetLevelStats();
    void RecordEnemyDefeat() { current_level_stats_.enemies_defeated++; }
    void RecordDamageTaken(uint32_t damage) { current_level_stats_.damage_taken += damage; }
    void RecordBonusCollected() { current_level_stats_.bonuses_collected++; }

    // Utility
    std::string GetDebugInfo() const;
    void Update(float deltaTime);

private:
    uint32_t total_score_ = 0;
    int combo_count_ = 0;
    int kill_streak_ = 0;
    int hits_ = 0;
    int misses_ = 0;

    // Multipliers
    float score_multiplier_ = 1.0f;
    float combo_multiplier_ = 1.0f;
    float difficulty_multiplier_ = 1.0f;
    float time_multiplier_ = 1.0f;
    float damage_penalty_ = 0.95f;

    // Combo decay
    float combo_decay_rate_ = 1.0f;  // seconds
    float combo_timer_ = 0.0f;

    // Base points for events
    std::unordered_map<int, uint32_t> event_base_points_;

    // Level statistics
    LevelStats current_level_stats_;

    // Helper methods
    void InitializeBasePoints();
    void UpdateComboDecay(float deltaTime);
};
