#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>

/**
 * @class UISystem
 * @brief Manages all UI rendering and display elements
 * 
 * Handles HUD elements, menus, overlays, and text rendering.
 * Provides abstraction over rendering backend.
 */
class UISystem {
public:
    enum class UIElement {
        // HUD
        Speedometer,
        GearIndicator,
        LapCounter,
        PositionDisplay,
        BestLapTime,
        CurrentLapTime,

        // Status
        DamageIndicator,
        EngineTemp,
        Throttle,
        Brake,
        Steering,

        // Notifications
        ObstacleWarning,
        PositionChange,
        LapComplete,

        // Menus
        PauseMenu,
        MainMenu,
        SettingsMenu,
        ControlsMenu,

        Count
    };

    enum class FontSize {
        Small,      // 12px
        Normal,     // 16px
        Large,      // 24px
        XLarge      // 32px
    };

    enum class Alignment {
        Left,
        Center,
        Right
    };

    struct TextOptions {
        FontSize size = FontSize::Normal;
        Alignment alignment = Alignment::Left;
        glm::vec3 color = glm::vec3(1.0f);  // White
        float opacity = 1.0f;
        bool bold = false;
        bool shadow = false;
    };

    struct UIPanel {
        std::string id;
        glm::vec2 position;
        glm::vec2 size;
        glm::vec4 background_color = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f);
        float border_width = 2.0f;
        glm::vec3 border_color = glm::vec3(1.0f);
        bool visible = true;
        bool has_background = true;
        bool has_border = true;
    };

    UISystem();
    ~UISystem();

    // Initialization
    bool Initialize(int screen_width, int screen_height);
    void Shutdown();

    // Screen management
    void SetScreenSize(int width, int height);
    int GetScreenWidth() const { return screen_width_; }
    int GetScreenHeight() const { return screen_height_; }

    // Text rendering
    void DrawText(const std::string& text, float x, float y,
                  const TextOptions& options = TextOptions());
    void DrawText(const std::string& text, glm::vec2 position,
                  const TextOptions& options = TextOptions());

    // Panel rendering
    void CreatePanel(const UIPanel& panel);
    void RemovePanel(const std::string& panel_id);
    void UpdatePanel(const UIPanel& panel);
    void DrawPanel(const std::string& panel_id);
    void ShowPanel(const std::string& panel_id);
    void HidePanel(const std::string& panel_id);

    // UI Elements
    void UpdateSpeedometer(float speed, float max_speed);
    void UpdateGearIndicator(int current_gear, int max_gear, bool is_automatic);
    void UpdateLapCounter(int current_lap, int total_laps);
    void UpdatePositionDisplay(int current_position, int total_racers);
    void UpdateLapTime(float current_time, float best_time);
    void UpdateDamageIndicator(float health_percentage);
    void UpdateThrottleBar(float throttle);
    void UpdateBrakeBar(float brake);
    void UpdateSteeringIndicator(float steering);

    // Notifications
    void ShowObstacleWarning(const std::string& obstacle_type);
    void ShowPositionChangeNotification(int new_position);
    void ShowLapCompleteNotification(int lap_number, float lap_time);
    void ClearNotification();

    // HUD visibility
    void ShowHUD();
    void HideHUD();
    bool IsHUDVisible() const { return hud_visible_; }

    // Menu management
    void ShowPauseMenu();
    void HidePauseMenu();
    void ShowMainMenu();
    void HideMainMenu();
    void ShowSettingsMenu();
    void HideSettingsMenu();

    // Rendering
    void BeginFrame();
    void EndFrame();
    void Render();  // Render all visible UI elements

    // Debug
    std::string GetDebugInfo() const;

private:
    struct SpeedometerData {
        float current_speed = 0.0f;
        float max_speed = 300.0f;
    } speedometer_data_;

    struct GearIndicatorData {
        int current_gear = 0;
        int max_gear = 6;
        bool is_automatic = true;
    } gear_indicator_data_;

    struct LapCounterData {
        int current_lap = 1;
        int total_laps = 3;
    } lap_counter_data_;

    struct PositionDisplayData {
        int current_position = 1;
        int total_racers = 4;
    } position_data_;

    struct LapTimeData {
        float current_time = 0.0f;
        float best_time = 0.0f;
    } lap_time_data_;

    struct DamageIndicatorData {
        float health = 100.0f;  // 0-100%
    } damage_data_;

    struct BarData {
        float throttle = 0.0f;
        float brake = 0.0f;
        float steering = 0.0f;
    } bar_data_;

    struct NotificationData {
        std::string text;
        float display_time = 3.0f;
        float elapsed_time = 0.0f;
        bool active = false;
    } notification_data_;

    int screen_width_ = 1920;
    int screen_height_ = 1080;
    bool hud_visible_ = true;
    bool frame_started_ = false;

    std::unordered_map<std::string, UIPanel> panels_;
    bool initialized_ = false;

    // Helper methods
    void RenderSpeedometer();
    void RenderGearIndicator();
    void RenderLapCounter();
    void RenderPositionDisplay();
    void RenderLapTime();
    void RenderDamageIndicator();
    void RenderBars();
    void RenderNotification();
    void UpdateNotification(float deltaTime);
    glm::vec2 AlignText(const std::string& text, glm::vec2 position,
                        Alignment alignment, FontSize size);
};
