#pragma once

#include "RenderSystem.h"
#include <memory>
#include <vector>

class RaceManager;  // Forward declaration
class Vehicle;      // Forward declaration
class Track;        // Forward declaration
class UISystem;     // Forward declaration

/**
 * @class RaceRenderer
 * @brief Handles rendering of race scenes including track, vehicles, and effects
 * 
 * Manages sprite generation, vehicle rendering, track visualization,
 * and visual effects during racing gameplay.
 */
class RaceRenderer {
public:
    enum class RenderLayer {
        Background = 0,
        Track = 100,
        Obstacles = 200,
        Vehicles = 300,
        Effects = 400,
        UI = 500
    };

    struct VehicleRenderData {
        const Vehicle* vehicle;
        Sprite sprite;
        std::vector<Sprite> trail_particles;  // Drift/speed trail
        bool is_player = false;
    };

    struct TrackRenderData {
        std::vector<Sprite> road_segments;
        std::vector<Sprite> barriers;
        std::vector<Sprite> decorations;
    };

    struct EffectRenderData {
        std::string effect_type;  // "smoke", "spark", "drift", etc.
        Sprite sprite;
        float lifetime = 0.0f;     // Current duration
        float max_lifetime = 0.0f; // Total duration
    };

    struct RaceRendererConfig {
        bool render_track = true;
        bool render_vehicles = true;
        bool render_effects = true;
        bool render_ui = true;
        bool show_vehicle_debug = false;     // Show bounding boxes
        bool show_track_debug = false;       // Show collision areas
        bool enable_motion_blur = false;
        bool enable_particle_effects = true;
        float particle_intensity = 1.0f;
    };

    RaceRenderer(RaceManager* race_manager, RenderSystem* render_system);
    ~RaceRenderer();

    // Initialization
    bool Initialize(const RaceRendererConfig& config);
    void Shutdown();

    // Rendering pipeline
    void BeginFrame();
    void RenderFrame();
    void EndFrame();

    // Track rendering
    void RenderTrack(const Track* track);
    void RenderTrackSegment(const glm::vec2& pos, float rotation, const std::string& segment_type);
    void RenderBarrier(const glm::vec2& pos, float rotation, const glm::vec2& size);
    void RenderTrackDecorations(const Track* track);

    // Vehicle rendering
    void RenderVehicles(const std::vector<Vehicle*>& vehicles);
    void RenderVehicle(const Vehicle* vehicle, bool is_player = false);
    void RenderVehicleSprite(const Vehicle* vehicle, const Sprite& sprite);
    void RenderVehicleTrail(const Vehicle* vehicle);
    void RenderVehicleDebug(const Vehicle* vehicle);  // Bounding box, velocity arrow

    // Effects
    void AddEffect(const std::string& effect_type, const glm::vec2& position,
                   float duration = 1.0f);
    void RenderEffects();
    void UpdateEffects(float deltaTime);
    void ClearEffects();

    // Camera control
    void UpdateCameraForPlayer(const Vehicle* player_vehicle);
    void SetCameraTarget(const glm::vec2& target);
    void SetCameraZoom(float zoom);

    // Configuration
    void SetConfig(const RaceRendererConfig& config) { config_ = config; }
    RaceRendererConfig GetConfig() const { return config_; }

    // Texture loading
    bool LoadAssets();
    void UnloadAssets();

    // Debug
    std::string GetDebugInfo() const;

private:
    RaceManager* race_manager_;      // Reference to race manager
    RenderSystem* render_system_;    // Reference to render system
    RaceRendererConfig config_;

    // Asset management
    bool assets_loaded_ = false;
    std::vector<std::string> loaded_textures_;

    // Rendering data
    std::vector<VehicleRenderData> vehicle_render_data_;
    std::vector<EffectRenderData> active_effects_;
    TrackRenderData track_render_data_;

    // Camera settings
    glm::vec2 camera_target_ = glm::vec2(0.0f);
    float camera_zoom_ = 1.0f;
    float camera_follow_distance_ = 100.0f;  // Offset from vehicle

    bool initialized_ = false;

    // Helper methods
    void LoadVehicleAssets();
    void LoadTrackAssets();
    void LoadEffectAssets();
    Sprite CreateVehicleSprite(const Vehicle* vehicle) const;
    Sprite CreateEffectSprite(const std::string& effect_type,
                             const glm::vec2& position, float opacity) const;
    void RenderSpriteLayer(RenderLayer layer);
    void ApplyMotionBlur(const Vehicle* vehicle);
    glm::vec3 GetVehicleColor(int vehicle_index) const;
};
