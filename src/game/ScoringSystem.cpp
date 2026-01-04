#include "ScoringSystem.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

ScoringSystem::ScoringSystem() {
    InitializeBasePoints();
    ResetScore();
}

void ScoringSystem::InitializeBasePoints() {
    event_base_points_[static_cast<int>(ScoreEvent::EnemyDefeat)] = 100;
    event_base_points_[static_cast<int>(ScoreEvent::ObstacleDestroy)] = 50;
    event_base_points_[static_cast<int>(ScoreEvent::PowerUpCollect)] = 25;
    event_base_points_[static_cast<int>(ScoreEvent::LevelComplete)] = 1000;
    event_base_points_[static_cast<int>(ScoreEvent::NoHitBonus)] = 500;
    event_base_points_[static_cast<int>(ScoreEvent::SpeedBonus)] = 200;
    event_base_points_[static_cast<int>(ScoreEvent::ComboKill)] = 150;
    event_base_points_[static_cast<int>(ScoreEvent::Perfect)] = 2000;
    event_base_points_[static_cast<int>(ScoreEvent::Custom)] = 0;
}

uint32_t ScoringSystem::AddScore(ScoreEvent event, uint32_t base_points) {
    uint32_t points = base_points > 0 ? base_points : GetBasePoints(event);

    // Apply multipliers
    float multiplier = score_multiplier_ * difficulty_multiplier_ * combo_multiplier_ * time_multiplier_;
    uint32_t final_points = static_cast<uint32_t>(points * multiplier);

    // Add special handling for combo
    if (event == ScoreEvent::EnemyDefeat || event == ScoreEvent::ComboKill) {
        IncrementCombo();
    }

    total_score_ += final_points;
    current_level_stats_.level_score += final_points;

    return final_points;
}

uint32_t ScoringSystem::AddScore(uint32_t points) {
    return AddScore(ScoreEvent::Custom, points);
}

uint32_t ScoringSystem::GetBasePoints(ScoreEvent event) const {
    auto it = event_base_points_.find(static_cast<int>(event));
    if (it != event_base_points_.end()) {
        return it->second;
    }
    return 0;
}

void ScoringSystem::SetBasePoints(ScoreEvent event, uint32_t points) {
    event_base_points_[static_cast<int>(event)] = points;
}

void ScoringSystem::IncrementCombo() {
    combo_count_++;
    combo_timer_ = 0.0f;
    combo_multiplier_ = 1.0f + (combo_count_ * 0.1f);  // 10% per combo level
}

void ScoringSystem::ResetCombo() {
    combo_count_ = 0;
    combo_timer_ = 0.0f;
    combo_multiplier_ = 1.0f;
}

uint32_t ScoringSystem::GetTimeBonus(float time_elapsed) const {
    if (time_elapsed <= 0.0f) return 0;

    // Bonus decreases over time (inverse)
    float bonus_multiplier = std::max(0.1f, 2.0f - (time_elapsed / 60.0f));
    return static_cast<uint32_t>(500 * bonus_multiplier);
}

float ScoringSystem::GetTimeBonusMultiplier(float time_elapsed) const {
    if (time_elapsed <= 0.0f) return 1.0f;
    return std::max(0.5f, 2.0f - (time_elapsed / 100.0f));
}

float ScoringSystem::GetAccuracy() const {
    int total = hits_ + misses_;
    if (total == 0) return 100.0f;
    return (static_cast<float>(hits_) / total) * 100.0f;
}

void ScoringSystem::IncrementKillStreak() {
    kill_streak_++;
}

void ScoringSystem::ResetKillStreak() {
    kill_streak_ = 0;
}

void ScoringSystem::ResetLevelStats() {
    current_level_stats_ = LevelStats();
}

void ScoringSystem::Update(float deltaTime) {
    UpdateComboDecay(deltaTime);
}

void ScoringSystem::UpdateComboDecay(float deltaTime) {
    if (combo_count_ > 0) {
        combo_timer_ += deltaTime;
        if (combo_timer_ >= combo_decay_rate_) {
            ResetCombo();
        }
    }
}

std::string ScoringSystem::GetDebugInfo() const {
    std::stringstream ss;
    ss << "ScoringSystem Debug Info\n";
    ss << "- Total Score: " << total_score_ << "\n";
    ss << "- Level Score: " << current_level_stats_.level_score << "\n";
    ss << "- Combo Count: " << combo_count_ << " (x" << std::fixed << std::setprecision(2) 
       << combo_multiplier_ << ")\n";
    ss << "- Kill Streak: " << kill_streak_ << "\n";
    ss << "- Accuracy: " << GetAccuracy() << "%\n";
    ss << "- Enemies Defeated: " << current_level_stats_.enemies_defeated << "\n";
    ss << "- Damage Taken: " << current_level_stats_.damage_taken << "\n";
    ss << "- Bonuses Collected: " << current_level_stats_.bonuses_collected << "\n";
    ss << "- Multipliers: Difficulty:" << difficulty_multiplier_ << " Time:" << time_multiplier_ << " Score:" << score_multiplier_;
    return ss.str();
}
