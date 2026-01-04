#include "UISystem.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

UISystem::UISystem() {
}

UISystem::~UISystem() {
    if (initialized_) {
        Shutdown();
    }
}

bool UISystem::Initialize(int screen_width, int screen_height) {
    if (initialized_) {
        return true;
    }

    screen_width_ = screen_width;
    screen_height_ = screen_height;
    hud_visible_ = true;

    // Initialize default panels
    UIPanel hud_panel;
    hud_panel.id = "hud_main";
    hud_panel.position = glm::vec2(20.0f, 20.0f);
    hud_panel.size = glm::vec2(300.0f, 200.0f);
    hud_panel.background_color = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f);
    CreatePanel(hud_panel);

    initialized_ = true;
    return true;
}

void UISystem::Shutdown() {
    panels_.clear();
    initialized_ = false;
}

void UISystem::SetScreenSize(int width, int height) {
    screen_width_ = width;
    screen_height_ = height;
}

void UISystem::DrawText(const std::string& text, float x, float y,
                        const TextOptions& options) {
    DrawText(text, glm::vec2(x, y), options);
}

void UISystem::DrawText(const std::string& text, glm::vec2 position,
                        const TextOptions& options) {
    if (!hud_visible_) return;

    // In a real implementation, this would render text using a font renderer
    // For now, this is a placeholder
}

void UISystem::CreatePanel(const UIPanel& panel) {
    panels_[panel.id] = panel;
}

void UISystem::RemovePanel(const std::string& panel_id) {
    panels_.erase(panel_id);
}

void UISystem::UpdatePanel(const UIPanel& panel) {
    auto it = panels_.find(panel.id);
    if (it != panels_.end()) {
        it->second = panel;
    }
}

void UISystem::DrawPanel(const std::string& panel_id) {
    auto it = panels_.find(panel_id);
    if (it != panels_.end() && it->second.visible) {
        const auto& panel = it->second;
        // Render panel background and border
        // This would be implemented with actual rendering
    }
}

void UISystem::ShowPanel(const std::string& panel_id) {
    auto it = panels_.find(panel_id);
    if (it != panels_.end()) {
        it->second.visible = true;
    }
}

void UISystem::HidePanel(const std::string& panel_id) {
    auto it = panels_.find(panel_id);
    if (it != panels_.end()) {
        it->second.visible = false;
    }
}

void UISystem::UpdateSpeedometer(float speed, float max_speed) {
    speedometer_data_.current_speed = speed;
    speedometer_data_.max_speed = max_speed;
}

void UISystem::UpdateGearIndicator(int current_gear, int max_gear, bool is_automatic) {
    gear_indicator_data_.current_gear = current_gear;
    gear_indicator_data_.max_gear = max_gear;
    gear_indicator_data_.is_automatic = is_automatic;
}

void UISystem::UpdateLapCounter(int current_lap, int total_laps) {
    lap_counter_data_.current_lap = current_lap;
    lap_counter_data_.total_laps = total_laps;
}

void UISystem::UpdatePositionDisplay(int current_position, int total_racers) {
    position_data_.current_position = current_position;
    position_data_.total_racers = total_racers;
}

void UISystem::UpdateLapTime(float current_time, float best_time) {
    lap_time_data_.current_time = current_time;
    lap_time_data_.best_time = best_time;
}

void UISystem::UpdateDamageIndicator(float health_percentage) {
    damage_data_.health = std::max(0.0f, std::min(100.0f, health_percentage));
}

void UISystem::UpdateThrottleBar(float throttle) {
    bar_data_.throttle = std::max(0.0f, std::min(1.0f, throttle));
}

void UISystem::UpdateBrakeBar(float brake) {
    bar_data_.brake = std::max(0.0f, std::min(1.0f, brake));
}

void UISystem::UpdateSteeringIndicator(float steering) {
    bar_data_.steering = std::max(-1.0f, std::min(1.0f, steering));
}

void UISystem::ShowObstacleWarning(const std::string& obstacle_type) {
    notification_data_.text = "Watch out for " + obstacle_type;
    notification_data_.display_time = 2.0f;
    notification_data_.elapsed_time = 0.0f;
    notification_data_.active = true;
}

void UISystem::ShowPositionChangeNotification(int new_position) {
    std::stringstream ss;
    ss << "You are in position " << new_position;
    notification_data_.text = ss.str();
    notification_data_.display_time = 2.5f;
    notification_data_.elapsed_time = 0.0f;
    notification_data_.active = true;
}

void UISystem::ShowLapCompleteNotification(int lap_number, float lap_time) {
    std::stringstream ss;
    ss << "Lap " << lap_number << " complete - ";
    ss << std::fixed << std::setprecision(2) << lap_time << "s";
    notification_data_.text = ss.str();
    notification_data_.display_time = 3.0f;
    notification_data_.elapsed_time = 0.0f;
    notification_data_.active = true;
}

void UISystem::ClearNotification() {
    notification_data_.active = false;
    notification_data_.elapsed_time = 0.0f;
}

void UISystem::ShowHUD() {
    hud_visible_ = true;
}

void UISystem::HideHUD() {
    hud_visible_ = false;
}

void UISystem::ShowPauseMenu() {
    ShowPanel("pause_menu");
}

void UISystem::HidePauseMenu() {
    HidePanel("pause_menu");
}

void UISystem::ShowMainMenu() {
    ShowPanel("main_menu");
}

void UISystem::HideMainMenu() {
    HidePanel("main_menu");
}

void UISystem::ShowSettingsMenu() {
    ShowPanel("settings_menu");
}

void UISystem::HideSettingsMenu() {
    HidePanel("settings_menu");
}

void UISystem::BeginFrame() {
    frame_started_ = true;
}

void UISystem::EndFrame() {
    frame_started_ = false;
}

void UISystem::Render() {
    if (!initialized_ || !frame_started_) {
        return;
    }

    if (hud_visible_) {
        RenderSpeedometer();
        RenderGearIndicator();
        RenderLapCounter();
        RenderPositionDisplay();
        RenderLapTime();
        RenderDamageIndicator();
        RenderBars();
    }

    RenderNotification();

    // Render all visible panels
    for (auto& panel : panels_) {
        DrawPanel(panel.first);
    }
}

void UISystem::RenderSpeedometer() {
    TextOptions options;
    options.size = FontSize::Large;
    options.alignment = Alignment::Left;
    options.color = glm::vec3(0.0f, 1.0f, 0.0f);  // Green

    std::stringstream ss;
    ss << std::fixed << std::setprecision(0) << speedometer_data_.current_speed << " km/h";
    DrawText(ss.str(), 50.0f, 50.0f, options);
}

void UISystem::RenderGearIndicator() {
    TextOptions options;
    options.size = FontSize::XLarge;
    options.alignment = Alignment::Center;
    options.color = glm::vec3(1.0f, 1.0f, 0.0f);  // Yellow

    std::stringstream ss;
    if (gear_indicator_data_.is_automatic) {
        ss << "A";
    } else {
        ss << gear_indicator_data_.current_gear;
    }
    DrawText(ss.str(), screen_width_ / 2.0f, 50.0f, options);
}

void UISystem::RenderLapCounter() {
    TextOptions options;
    options.size = FontSize::Large;
    options.alignment = Alignment::Right;
    options.color = glm::vec3(0.0f, 1.0f, 1.0f);  // Cyan

    std::stringstream ss;
    ss << "Lap " << lap_counter_data_.current_lap << " / " << lap_counter_data_.total_laps;
    DrawText(ss.str(), screen_width_ - 50.0f, 50.0f, options);
}

void UISystem::RenderPositionDisplay() {
    TextOptions options;
    options.size = FontSize::Large;
    options.alignment = Alignment::Center;
    options.color = glm::vec3(1.0f, 0.5f, 0.0f);  // Orange

    std::stringstream ss;
    ss << "P" << position_data_.current_position << " / " << position_data_.total_racers;
    DrawText(ss.str(), 50.0f, 150.0f, options);
}

void UISystem::RenderLapTime() {
    TextOptions options;
    options.size = FontSize::Normal;
    options.alignment = Alignment::Left;
    options.color = glm::vec3(1.0f, 1.0f, 1.0f);  // White

    std::stringstream ss;
    ss << "Current: " << std::fixed << std::setprecision(2) << lap_time_data_.current_time << "s";
    DrawText(ss.str(), 50.0f, 200.0f, options);

    if (lap_time_data_.best_time > 0.0f) {
        ss.str("");
        ss.clear();
        ss << "Best: " << lap_time_data_.best_time << "s";
        DrawText(ss.str(), 50.0f, 230.0f, options);
    }
}

void UISystem::RenderDamageIndicator() {
    TextOptions options;
    options.size = FontSize::Normal;
    options.alignment = Alignment::Left;

    // Color changes based on health
    if (damage_data_.health > 66.0f) {
        options.color = glm::vec3(0.0f, 1.0f, 0.0f);  // Green
    } else if (damage_data_.health > 33.0f) {
        options.color = glm::vec3(1.0f, 1.0f, 0.0f);  // Yellow
    } else {
        options.color = glm::vec3(1.0f, 0.0f, 0.0f);  // Red
    }

    std::stringstream ss;
    ss << "Health: " << std::fixed << std::setprecision(0) << damage_data_.health << "%";
    DrawText(ss.str(), 50.0f, screen_height_ - 100.0f, options);
}

void UISystem::RenderBars() {
    // Render throttle, brake, and steering indicators
    // This would be rendered as graphical bars
}

void UISystem::RenderNotification() {
    if (!notification_data_.active) return;

    TextOptions options;
    options.size = FontSize::Large;
    options.alignment = Alignment::Center;
    options.color = glm::vec3(1.0f, 1.0f, 0.0f);  // Yellow
    options.opacity = 1.0f - (notification_data_.elapsed_time / notification_data_.display_time);

    DrawText(notification_data_.text, screen_width_ / 2.0f, 100.0f, options);
}

void UISystem::UpdateNotification(float deltaTime) {
    if (!notification_data_.active) return;

    notification_data_.elapsed_time += deltaTime;
    if (notification_data_.elapsed_time >= notification_data_.display_time) {
        notification_data_.active = false;
    }
}

glm::vec2 UISystem::AlignText(const std::string& text, glm::vec2 position,
                               Alignment alignment, FontSize size) {
    // In a real implementation, this would calculate text width and adjust position
    // For now, return the position as-is
    return position;
}

std::string UISystem::GetDebugInfo() const {
    std::stringstream ss;
    ss << "UISystem Debug Info\n";
    ss << "- Initialized: " << (initialized_ ? "Yes" : "No") << "\n";
    ss << "- Screen: " << screen_width_ << "x" << screen_height_ << "\n";
    ss << "- HUD Visible: " << (hud_visible_ ? "Yes" : "No") << "\n";
    ss << "- Speed: " << std::fixed << std::setprecision(1) << speedometer_data_.current_speed << " km/h\n";
    ss << "- Gear: " << gear_indicator_data_.current_gear << "\n";
    ss << "- Lap: " << lap_counter_data_.current_lap << " / " << lap_counter_data_.total_laps << "\n";
    ss << "- Position: " << position_data_.current_position << " / " << position_data_.total_racers << "\n";
    ss << "- Health: " << damage_data_.health << "%\n";
    ss << "- Panels: " << panels_.size() << "\n";
    return ss.str();
}
