#pragma once

#include "GameObject.h"
#include "Player.h"
#include "Enemy.h"
#include "Obstacle.h"
#include "PowerUp.h"
#include <memory>
#include <vector>
#include <unordered_map>

/**
 * @class GameManager
 * @brief Central manager for game state, entities, and gameplay logic
 * 
 * Manages the game loop, entity lifecycle, collision detection,
 * scoring, and game rules. Acts as the main coordinator for all game systems.
 */
class GameManager {
public:
    enum class GameState {
        Uninitialized,  // Not started
        Playing,        // Active gameplay
        Paused,         // Game paused
        LevelComplete,  // Level finished successfully
        GameOver,       // Player died
        Victory         // All levels completed
    };

    enum class Difficulty {
        Easy,
        Normal,
        Hard,
        Extreme
    };

    GameManager();
    ~GameManager();

    // Lifecycle
    bool Initialize();
    void Shutdown();
    void Update(float deltaTime);
    void Render();

    // Game state
    GameState GetGameState() const { return game_state_; }
    void SetGameState(GameState state);
    bool IsGameOver() const;
    bool IsLevelComplete() const;
    bool IsGamePaused() const;

    // Game control
    void Play();
    void Pause();
    void Resume();
    void Restart();
    void QuitGame();

    // Difficulty
    Difficulty GetDifficulty() const { return difficulty_; }
    void SetDifficulty(Difficulty diff) { difficulty_ = diff; }

    // Player management
    Player* GetPlayer() const { return player_.get(); }
    void SetPlayer(std::shared_ptr<Player> player);
    bool HasPlayer() const { return player_ != nullptr; }

    // Entity management
    void AddEntity(std::shared_ptr<GameObject> entity);
    void RemoveEntity(std::shared_ptr<GameObject> entity);
    void ClearEntities();
    std::vector<std::shared_ptr<GameObject>> GetEntities() const;
    std::vector<std::shared_ptr<Enemy>> GetEnemies() const;
    std::vector<std::shared_ptr<Obstacle>> GetObstacles() const;
    std::vector<std::shared_ptr<PowerUp>> GetPowerUps() const;

    // Entity spawning
    std::shared_ptr<Enemy> SpawnEnemy(const glm::vec3& position, const std::string& name = "Enemy");
    std::shared_ptr<Obstacle> SpawnObstacle(const glm::vec3& position, Obstacle::ObstacleType type = Obstacle::ObstacleType::Wall, const std::string& name = "Obstacle");
    std::shared_ptr<PowerUp> SpawnPowerUp(const glm::vec3& position, PowerUp::PowerUpType type = PowerUp::PowerUpType::HealthRestore, const std::string& name = "PowerUp");

    // Collision system
    void UpdateCollisions();
    std::vector<GameObject*> GetCollidingObjects(const GameObject& object) const;
    bool CheckCollision(const GameObject& a, const GameObject& b) const;

    // Scoring system
    uint32_t GetScore() const { return player_ ? player_->GetScore() : 0; }
    void AddScore(uint32_t points);
    void ResetScore();

    // Lives and health
    int GetLives() const { return player_ ? player_->GetLives() : 0; }
    void SetLives(int lives);
    bool IsPlayerAlive() const { return player_ && player_->IsAlive(); }

    // Level management
    int GetCurrentLevel() const { return current_level_; }
    void SetCurrentLevel(int level) { current_level_ = level; }
    void NextLevel();
    void PreviousLevel();
    int GetMaxLevel() const { return max_level_; }

    // Game time
    float GetElapsedTime() const { return elapsed_time_; }
    float GetLevelTime() const { return level_time_; }
    void ResetLevelTime() { level_time_ = 0.0f; }

    // Difficulty scaling
    float GetDifficultyMultiplier() const;
    float GetEnemyDamageMultiplier() const;
    float GetEnemySpeedMultiplier() const;
    float GetScoreMultiplier() const;

    // Game rules
    bool CheckWinCondition() const;
    bool CheckLoseCondition() const;
    void ApplyGameRules(float deltaTime);

    // Debug info
    std::string GetDebugInfo() const;
    int GetEntityCount() const { return entities_.size(); }
    int GetEnemyCount() const;
    int GetObstacleCount() const;
    int GetPowerUpCount() const;

private:
    GameState game_state_ = GameState::Uninitialized;
    Difficulty difficulty_ = Difficulty::Normal;

    // Entities
    std::shared_ptr<Player> player_;
    std::vector<std::shared_ptr<GameObject>> entities_;
    std::vector<std::shared_ptr<Enemy>> enemies_;
    std::vector<std::shared_ptr<Obstacle>> obstacles_;
    std::vector<std::shared_ptr<PowerUp>> powerups_;

    // Game state
    int current_level_ = 1;
    int max_level_ = 5;  // Can be configured
    float elapsed_time_ = 0.0f;
    float level_time_ = 0.0f;
    bool paused_ = false;

    // Game timing
    float time_scale_ = 1.0f;

    // Helper methods
    void UpdateEntities(float deltaTime);
    void UpdatePlayer(float deltaTime);
    void UpdateEnemies(float deltaTime);
    void UpdateObstacles(float deltaTime);
    void UpdatePowerUps(float deltaTime);
    void CleanupDeadEntities();
    void CheckAndApplyCollisions();
    void OnPlayerDeath();
    void OnLevelComplete();
    void OnGameOver();
};
