#include "GameManager.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

GameManager::GameManager() {}

GameManager::~GameManager() {
    Shutdown();
}

bool GameManager::Initialize() {
    if (game_state_ != GameState::Uninitialized) {
        return false;
    }

    // Create player if not set
    if (!player_) {
        player_ = std::make_shared<Player>("MainPlayer");
        player_->Initialize();
        player_->SetPosition(0.0f, 1.0f, 0.0f);
    }

    game_state_ = GameState::Playing;
    elapsed_time_ = 0.0f;
    level_time_ = 0.0f;
    return true;
}

void GameManager::Shutdown() {
    ClearEntities();
    if (player_) {
        player_->Shutdown();
        player_.reset();
    }
    game_state_ = GameState::Uninitialized;
}

void GameManager::Update(float deltaTime) {
    if (game_state_ == GameState::Uninitialized) return;

    // Apply time scale
    float scaled_deltaTime = deltaTime * time_scale_;

    // Update timing
    if (!paused_) {
        elapsed_time_ += scaled_deltaTime;
        level_time_ += scaled_deltaTime;
    }

    // Update all entities
    if (!paused_) {
        UpdateEntities(scaled_deltaTime);
        UpdateCollisions();
    }

    // Apply game rules
    ApplyGameRules(scaled_deltaTime);

    // Check game conditions
    if (CheckWinCondition()) {
        OnLevelComplete();
    } else if (CheckLoseCondition()) {
        OnGameOver();
    }
}

void GameManager::Render() {
    // Rendering handled by game engine
}

void GameManager::SetGameState(GameState state) {
    GameState old_state = game_state_;
    game_state_ = state;

    if (state == GameState::LevelComplete) {
        OnLevelComplete();
    } else if (state == GameState::GameOver) {
        OnGameOver();
    }
}

bool GameManager::IsGameOver() const {
    return game_state_ == GameState::GameOver;
}

bool GameManager::IsLevelComplete() const {
    return game_state_ == GameState::LevelComplete;
}

bool GameManager::IsGamePaused() const {
    return paused_;
}

void GameManager::Play() {
    if (game_state_ == GameState::Uninitialized) {
        Initialize();
    } else if (game_state_ != GameState::Playing) {
        game_state_ = GameState::Playing;
    }
    paused_ = false;
}

void GameManager::Pause() {
    if (game_state_ == GameState::Playing) {
        paused_ = true;
    }
}

void GameManager::Resume() {
    paused_ = false;
}

void GameManager::Restart() {
    Shutdown();
    Initialize();
}

void GameManager::QuitGame() {
    game_state_ = GameState::GameOver;
}

void GameManager::SetPlayer(std::shared_ptr<Player> player) {
    if (player_) {
        player_->Shutdown();
    }
    player_ = player;
    if (player_) {
        player_->Initialize();
    }
}

void GameManager::AddEntity(std::shared_ptr<GameObject> entity) {
    if (!entity) return;

    entities_.push_back(entity);
    entity->Initialize();

    // Sort by type
    if (auto enemy = std::dynamic_pointer_cast<Enemy>(entity)) {
        enemies_.push_back(enemy);
    } else if (auto obstacle = std::dynamic_pointer_cast<Obstacle>(entity)) {
        obstacles_.push_back(obstacle);
    } else if (auto powerup = std::dynamic_pointer_cast<PowerUp>(entity)) {
        powerups_.push_back(powerup);
    }
}

void GameManager::RemoveEntity(std::shared_ptr<GameObject> entity) {
    auto it = std::find(entities_.begin(), entities_.end(), entity);
    if (it != entities_.end()) {
        entity->Shutdown();
        entities_.erase(it);
    }
}

void GameManager::ClearEntities() {
    for (auto& entity : entities_) {
        entity->Shutdown();
    }
    entities_.clear();
    enemies_.clear();
    obstacles_.clear();
    powerups_.clear();
}

std::vector<std::shared_ptr<GameObject>> GameManager::GetEntities() const {
    return entities_;
}

std::vector<std::shared_ptr<Enemy>> GameManager::GetEnemies() const {
    return enemies_;
}

std::vector<std::shared_ptr<Obstacle>> GameManager::GetObstacles() const {
    return obstacles_;
}

std::vector<std::shared_ptr<PowerUp>> GameManager::GetPowerUps() const {
    return powerups_;
}

std::shared_ptr<Enemy> GameManager::SpawnEnemy(const glm::vec3& position, const std::string& name) {
    auto enemy = std::make_shared<Enemy>(name);
    enemy->SetPosition(position);
    AddEntity(enemy);
    return enemy;
}

std::shared_ptr<Obstacle> GameManager::SpawnObstacle(const glm::vec3& position, Obstacle::ObstacleType type, const std::string& name) {
    auto obstacle = std::make_shared<Obstacle>(name, type);
    obstacle->SetPosition(position);
    AddEntity(obstacle);
    return obstacle;
}

std::shared_ptr<PowerUp> GameManager::SpawnPowerUp(const glm::vec3& position, PowerUp::PowerUpType type, const std::string& name) {
    auto powerup = std::make_shared<PowerUp>(name, type);
    powerup->SetPosition(position);
    AddEntity(powerup);
    return powerup;
}

void GameManager::UpdateCollisions() {
    // Check all entities for collisions
    for (size_t i = 0; i < entities_.size(); ++i) {
        for (size_t j = i + 1; j < entities_.size(); ++j) {
            if (CheckCollision(*entities_[i], *entities_[j])) {
                entities_[i]->OnCollisionEnter(*entities_[j]);
                entities_[j]->OnCollisionEnter(*entities_[i]);
            }
        }
    }

    // Check player collisions
    if (player_) {
        for (auto& entity : entities_) {
            if (CheckCollision(*player_, *entity)) {
                player_->OnCollisionEnter(*entity);
                entity->OnCollisionEnter(*player_);
            }
        }
    }
}

std::vector<GameObject*> GameManager::GetCollidingObjects(const GameObject& object) const {
    std::vector<GameObject*> colliding;
    for (auto& entity : entities_) {
        if (entity.get() != &object && CheckCollision(object, *entity)) {
            colliding.push_back(entity.get());
        }
    }
    return colliding;
}

bool GameManager::CheckCollision(const GameObject& a, const GameObject& b) const {
    if (!a.HasCollision() || !b.HasCollision()) {
        return false;
    }
    return a.IsCollidingWith(b);
}

void GameManager::AddScore(uint32_t points) {
    if (player_) {
        uint32_t adjusted_points = static_cast<uint32_t>(points * GetScoreMultiplier());
        player_->AddScore(adjusted_points);
    }
}

void GameManager::ResetScore() {
    if (player_) {
        player_->ResetScore();
    }
}

void GameManager::SetLives(int lives) {
    if (player_) {
        player_->SetLives(lives);
    }
}

void GameManager::NextLevel() {
    current_level_ = std::min(current_level_ + 1, max_level_);
    level_time_ = 0.0f;
}

void GameManager::PreviousLevel() {
    current_level_ = std::max(1, current_level_ - 1);
    level_time_ = 0.0f;
}

float GameManager::GetDifficultyMultiplier() const {
    switch (difficulty_) {
        case Difficulty::Easy:
            return 0.75f;
        case Difficulty::Normal:
            return 1.0f;
        case Difficulty::Hard:
            return 1.5f;
        case Difficulty::Extreme:
            return 2.0f;
    }
    return 1.0f;
}

float GameManager::GetEnemyDamageMultiplier() const {
    return GetDifficultyMultiplier();
}

float GameManager::GetEnemySpeedMultiplier() const {
    return GetDifficultyMultiplier() * 0.8f;  // Speed increases less than damage
}

float GameManager::GetScoreMultiplier() const {
    return GetDifficultyMultiplier();
}

bool GameManager::CheckWinCondition() const {
    if (!player_ || game_state_ != GameState::Playing) {
        return false;
    }

    // Win if all enemies are defeated and player is alive
    bool all_enemies_defeated = enemies_.empty() || 
        std::all_of(enemies_.begin(), enemies_.end(), 
                   [](const auto& e) { return !e->IsAlive(); });

    return all_enemies_defeated && player_->IsAlive();
}

bool GameManager::CheckLoseCondition() const {
    if (!player_ || game_state_ != GameState::Playing) {
        return false;
    }

    // Lose if player is dead
    return !player_->IsAlive() || player_->IsGameOver();
}

void GameManager::ApplyGameRules(float deltaTime) {
    if (!player_) return;

    // Scale enemy difficulty
    for (auto& enemy : enemies_) {
        if (enemy && enemy->IsActive()) {
            // Difficulty scaling applied when damage is calculated
        }
    }
}

void GameManager::UpdateEntities(float deltaTime) {
    if (player_) {
        player_->Update(deltaTime);
    }

    for (auto& entity : entities_) {
        if (entity && entity->IsActive()) {
            entity->Update(deltaTime);
        }
    }
}

void GameManager::UpdatePlayer(float deltaTime) {
    if (player_) {
        player_->Update(deltaTime);
    }
}

void GameManager::UpdateEnemies(float deltaTime) {
    for (auto& enemy : enemies_) {
        if (enemy && enemy->IsActive()) {
            enemy->Update(deltaTime);
        }
    }
}

void GameManager::UpdateObstacles(float deltaTime) {
    for (auto& obstacle : obstacles_) {
        if (obstacle && obstacle->IsActive()) {
            obstacle->Update(deltaTime);
        }
    }
}

void GameManager::UpdatePowerUps(float deltaTime) {
    for (auto& powerup : powerups_) {
        if (powerup && powerup->IsActive()) {
            powerup->Update(deltaTime);
        }
    }
}

void GameManager::CleanupDeadEntities() {
    auto it = entities_.begin();
    while (it != entities_.end()) {
        if (!(*it)->IsActive() || !(*it)->IsAlive()) {
            (*it)->Shutdown();
            it = entities_.erase(it);
        } else {
            ++it;
        }
    }
}

void GameManager::OnPlayerDeath() {
    if (player_) {
        player_->LoseLife();
        if (player_->IsGameOver()) {
            game_state_ = GameState::GameOver;
        }
    }
}

void GameManager::OnLevelComplete() {
    game_state_ = GameState::LevelComplete;
    if (current_level_ < max_level_) {
        NextLevel();
    } else {
        game_state_ = GameState::Victory;
    }
}

void GameManager::OnGameOver() {
    game_state_ = GameState::GameOver;
}

int GameManager::GetEnemyCount() const {
    return std::count_if(enemies_.begin(), enemies_.end(),
                        [](const auto& e) { return e && e->IsActive(); });
}

int GameManager::GetObstacleCount() const {
    return std::count_if(obstacles_.begin(), obstacles_.end(),
                        [](const auto& o) { return o && o->IsActive(); });
}

int GameManager::GetPowerUpCount() const {
    return std::count_if(powerups_.begin(), powerups_.end(),
                        [](const auto& p) { return p && p->IsActive(); });
}

std::string GameManager::GetDebugInfo() const {
    std::stringstream ss;
    ss << "GameManager Debug Info\n";
    ss << "- State: ";
    switch (game_state_) {
        case GameState::Uninitialized:
            ss << "Uninitialized";
            break;
        case GameState::Playing:
            ss << "Playing";
            break;
        case GameState::Paused:
            ss << "Paused";
            break;
        case GameState::LevelComplete:
            ss << "LevelComplete";
            break;
        case GameState::GameOver:
            ss << "GameOver";
            break;
        case GameState::Victory:
            ss << "Victory";
            break;
    }
    ss << "\n";
    ss << "- Level: " << current_level_ << "/" << max_level_ << "\n";
    ss << "- Difficulty: ";
    switch (difficulty_) {
        case Difficulty::Easy:
            ss << "Easy";
            break;
        case Difficulty::Normal:
            ss << "Normal";
            break;
        case Difficulty::Hard:
            ss << "Hard";
            break;
        case Difficulty::Extreme:
            ss << "Extreme";
            break;
    }
    ss << "\n";
    ss << "- Score: " << GetScore() << "\n";
    ss << "- Lives: " << GetLives() << "\n";
    ss << "- Elapsed Time: " << std::fixed << std::setprecision(2) << elapsed_time_ << "s\n";
    ss << "- Level Time: " << level_time_ << "s\n";
    ss << "- Entities: " << GetEntityCount() << " (E:" << GetEnemyCount() 
       << " O:" << GetObstacleCount() << " P:" << GetPowerUpCount() << ")\n";
    ss << "- Paused: " << (paused_ ? "Yes" : "No");
    return ss.str();
}
