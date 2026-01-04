#include "RaceRenderer.h"
#include "../game/RaceManager.h"
#include "../game/Vehicle.h"
#include "../game/Track.h"
#include "../ui/UISystem.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

RaceRenderer::RaceRenderer(RaceManager* race_manager, RenderSystem* render_system)
    : race_manager_(race_manager), render_system_(render_system) {
}

RaceRenderer::~RaceRenderer() {
    Shutdown();
}

bool RaceRenderer::Initialize(const RaceRendererConfig& config) {
    if (!race_manager_ || !render_system_) {
        return false;
    }

    config_ = config;

    if (!LoadAssets()) {
        return false;
    }

    initialized_ = true;
    return true;
}

void RaceRenderer::Shutdown() {
    UnloadAssets();
    vehicle_render_data_.clear();
    active_effects_.clear();
    initialized_ = false;
}

void RaceRenderer::BeginFrame() {
    if (render_system_) {
        render_system_->BeginFrame();
        render_system_->BeginBatch();
    }
}

void RaceRenderer::RenderFrame() {
    if (!initialized_ || !render_system_) return;

    // Render layers in order
    RenderSpriteLayer(RenderLayer::Background);
    RenderSpriteLayer(RenderLayer::Track);
    RenderSpriteLayer(RenderLayer::Obstacles);
    RenderSpriteLayer(RenderLayer::Vehicles);
    RenderSpriteLayer(RenderLayer::Effects);
    RenderSpriteLayer(RenderLayer::UI);
}

void RaceRenderer::EndFrame() {
    if (render_system_) {
        render_system_->EndBatch();
        render_system_->EndFrame();
        render_system_->Present();
    }
}

void RaceRenderer::RenderTrack(const Track* track) {
    if (!track || !render_system_ || !config_.render_track) {
        return;
    }

    // Clear previous track data
    track_render_data_.road_segments.clear();
    track_render_data_.barriers.clear();
    track_render_data_.decorations.clear();

    // Get track segments and create sprites
    auto segments = track->GetTrackSegments();
    for (const auto& segment : segments) {
        RenderTrackSegment(segment.position, segment.rotation, "road_segment");
    }

    // Render barriers
    auto barriers = track->GetBarriers();
    for (const auto& barrier : barriers) {
        RenderBarrier(barrier.position, barrier.rotation, barrier.size);
    }

    // Render decorations
    RenderTrackDecorations(track);
}

void RaceRenderer::RenderTrackSegment(const glm::vec2& pos, float rotation, const std::string& segment_type) {
    Sprite segment_sprite;
    segment_sprite.texture_id = segment_type;
    segment_sprite.position = pos;
    segment_sprite.rotation = rotation;
    segment_sprite.size = glm::vec2(128.0f, 128.0f);
    segment_sprite.layer = static_cast<int>(RenderLayer::Track);
    segment_sprite.color = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);

    track_render_data_.road_segments.push_back(segment_sprite);
    render_system_->DrawSprite(segment_sprite);
}

void RaceRenderer::RenderBarrier(const glm::vec2& pos, float rotation, const glm::vec2& size) {
    Sprite barrier_sprite;
    barrier_sprite.texture_id = "barrier";
    barrier_sprite.position = pos;
    barrier_sprite.rotation = rotation;
    barrier_sprite.size = size;
    barrier_sprite.layer = static_cast<int>(RenderLayer::Obstacles);
    barrier_sprite.color = glm::vec4(1.0f, 0.2f, 0.2f, 1.0f);  // Red barriers

    track_render_data_.barriers.push_back(barrier_sprite);
    render_system_->DrawSprite(barrier_sprite);
}

void RaceRenderer::RenderTrackDecorations(const Track* track) {
    // Render track decorations (trees, signs, etc.)
    if (!track || track_render_data_.decorations.empty()) {
        return;
    }

    for (const auto& decoration : track_render_data_.decorations) {
        render_system_->DrawSprite(decoration);
    }
}

void RaceRenderer::RenderVehicles(const std::vector<Vehicle*>& vehicles) {
    if (!config_.render_vehicles) return;

    vehicle_render_data_.clear();

    // Determine player vehicle
    const Vehicle* player_vehicle = race_manager_ ? race_manager_->GetPlayerVehicle() : nullptr;

    for (const auto& vehicle : vehicles) {
        if (!vehicle) continue;

        bool is_player = (vehicle == player_vehicle);
        RenderVehicle(vehicle, is_player);
    }

    // Sort vehicles by distance to camera (for proper z-ordering)
    if (render_system_) {
        glm::vec2 camera_pos = render_system_->GetCamera().position;
        std::sort(vehicle_render_data_.begin(), vehicle_render_data_.end(),
                  [camera_pos](const VehicleRenderData& a, const VehicleRenderData& b) {
                      float dist_a = glm::distance(a.vehicle->GetPosition(), camera_pos);
                      float dist_b = glm::distance(b.vehicle->GetPosition(), camera_pos);
                      return dist_a > dist_b;  // Far to near (painter's algorithm)
                  });
    }
}

void RaceRenderer::RenderVehicle(const Vehicle* vehicle, bool is_player) {
    if (!vehicle || !render_system_) return;

    VehicleRenderData render_data;
    render_data.vehicle = vehicle;
    render_data.is_player = is_player;

    // Create vehicle sprite
    Sprite vehicle_sprite = CreateVehicleSprite(vehicle);
    render_data.sprite = vehicle_sprite;

    // Render vehicle
    RenderVehicleSprite(vehicle, vehicle_sprite);

    // Render trail if moving fast
    if (vehicle->GetSpeed() > 50.0f && config_.enable_particle_effects) {
        RenderVehicleTrail(vehicle);
    }

    // Render debug info if enabled
    if (config_.show_vehicle_debug) {
        RenderVehicleDebug(vehicle);
    }

    vehicle_render_data_.push_back(render_data);
}

void RaceRenderer::RenderVehicleSprite(const Vehicle* vehicle, const Sprite& sprite) {
    if (!render_system_) return;

    Sprite render_sprite = sprite;
    render_sprite.layer = static_cast<int>(RenderLayer::Vehicles);
    render_system_->DrawSprite(render_sprite);
}

void RaceRenderer::RenderVehicleTrail(const Vehicle* vehicle) {
    if (!vehicle || !config_.enable_particle_effects) return;

    // Create trail particles behind vehicle
    glm::vec2 vehicle_pos = vehicle->GetPosition();
    glm::vec2 velocity = vehicle->GetVelocity();
    float speed = glm::length(velocity);

    // Trail opacity based on speed
    float opacity = std::min(1.0f, speed / vehicle->GetMaxSpeed()) * config_.particle_intensity;

    Sprite trail_sprite;
    trail_sprite.texture_id = "particle_smoke";
    trail_sprite.position = vehicle_pos - glm::normalize(velocity) * 20.0f;
    trail_sprite.size = glm::vec2(16.0f, 16.0f);
    trail_sprite.opacity = opacity * 0.7f;
    trail_sprite.layer = static_cast<int>(RenderLayer::Effects);
    trail_sprite.color = glm::vec4(0.5f, 0.5f, 0.5f, opacity);

    render_system_->DrawSprite(trail_sprite);
}

void RaceRenderer::RenderVehicleDebug(const Vehicle* vehicle) {
    // Render debug information (bounding box, velocity, etc.)
    if (!config_.show_vehicle_debug || !vehicle) return;

    glm::vec2 vehicle_pos = vehicle->GetPosition();
    glm::vec2 velocity = vehicle->GetVelocity();

    // In a real implementation, we would draw:
    // - Bounding box around vehicle
    // - Velocity vector arrow
    // - Position marker
}

void RaceRenderer::AddEffect(const std::string& effect_type, const glm::vec2& position,
                            float duration) {
    EffectRenderData effect;
    effect.effect_type = effect_type;
    effect.sprite = CreateEffectSprite(effect_type, position, 1.0f);
    effect.lifetime = 0.0f;
    effect.max_lifetime = duration;

    active_effects_.push_back(effect);
}

void RaceRenderer::RenderEffects() {
    if (!config_.render_effects || !render_system_) return;

    for (auto& effect : active_effects_) {
        if (effect.lifetime < effect.max_lifetime) {
            // Calculate opacity based on lifetime
            float progress = effect.lifetime / effect.max_lifetime;
            effect.sprite.opacity = 1.0f - progress;  // Fade out
            effect.sprite.layer = static_cast<int>(RenderLayer::Effects);

            render_system_->DrawSprite(effect.sprite);
        }
    }
}

void RaceRenderer::UpdateEffects(float deltaTime) {
    // Update effect lifetimes
    for (auto& effect : active_effects_) {
        effect.lifetime += deltaTime;
    }

    // Remove expired effects
    auto it = std::remove_if(active_effects_.begin(), active_effects_.end(),
                             [](const EffectRenderData& effect) {
                                 return effect.lifetime >= effect.max_lifetime;
                             });
    active_effects_.erase(it, active_effects_.end());
}

void RaceRenderer::ClearEffects() {
    active_effects_.clear();
}

void RaceRenderer::UpdateCameraForPlayer(const Vehicle* player_vehicle) {
    if (!player_vehicle || !render_system_) return;

    glm::vec2 vehicle_pos = player_vehicle->GetPosition();
    glm::vec2 velocity = player_vehicle->GetVelocity();

    // Camera offset in direction of travel
    glm::vec2 camera_target = vehicle_pos;
    if (glm::length(velocity) > 0.1f) {
        glm::vec2 forward = glm::normalize(velocity);
        camera_target += forward * camera_follow_distance_;
    }

    // Update camera
    RenderSystem::Camera camera = render_system_->GetCamera();
    camera.follow_target = true;
    camera.target = camera_target;
    render_system_->SetCamera(camera);
    render_system_->UpdateCamera(camera_target);
}

void RaceRenderer::SetCameraTarget(const glm::vec2& target) {
    if (render_system_) {
        RenderSystem::Camera camera = render_system_->GetCamera();
        camera.target = target;
        render_system_->SetCamera(camera);
    }
}

void RaceRenderer::SetCameraZoom(float zoom) {
    if (render_system_) {
        render_system_->SetCameraZoom(zoom);
    }
}

bool RaceRenderer::LoadAssets() {
    if (!render_system_) return false;

    // Load vehicle textures
    LoadVehicleAssets();

    // Load track textures
    LoadTrackAssets();

    // Load effect textures
    LoadEffectAssets();

    assets_loaded_ = true;
    return true;
}

void RaceRenderer::UnloadAssets() {
    for (const auto& texture_id : loaded_textures_) {
        if (render_system_) {
            render_system_->UnloadTexture(texture_id);
        }
    }
    loaded_textures_.clear();
    assets_loaded_ = false;
}

void RaceRenderer::LoadVehicleAssets() {
    std::vector<std::string> vehicle_textures = {
        "vehicle_red", "vehicle_blue", "vehicle_green", "vehicle_yellow",
        "vehicle_orange", "vehicle_purple", "vehicle_pink", "vehicle_cyan"
    };

    for (const auto& texture : vehicle_textures) {
        if (render_system_) {
            render_system_->LoadTexture(texture, "");
            loaded_textures_.push_back(texture);
        }
    }
}

void RaceRenderer::LoadTrackAssets() {
    std::vector<std::string> track_textures = {
        "road_segment", "road_corner", "barrier", "finish_line",
        "road_straight", "road_curve_left", "road_curve_right"
    };

    for (const auto& texture : track_textures) {
        if (render_system_) {
            render_system_->LoadTexture(texture, "");
            loaded_textures_.push_back(texture);
        }
    }
}

void RaceRenderer::LoadEffectAssets() {
    std::vector<std::string> effect_textures = {
        "particle_smoke", "particle_dust", "particle_spark",
        "particle_explosion", "particle_drift"
    };

    for (const auto& texture : effect_textures) {
        if (render_system_) {
            render_system_->LoadTexture(texture, "");
            loaded_textures_.push_back(texture);
        }
    }
}

Sprite RaceRenderer::CreateVehicleSprite(const Vehicle* vehicle) const {
    Sprite sprite;
    sprite.texture_id = "vehicle_red";  // Default color
    sprite.position = vehicle->GetPosition();
    sprite.rotation = vehicle->GetRotation();
    sprite.size = glm::vec2(32.0f, 64.0f);  // Vehicle size
    sprite.color = glm::vec4(1.0f);  // White (will be tinted by texture)
    sprite.opacity = 1.0f;
    sprite.layer = static_cast<int>(RenderLayer::Vehicles);
    sprite.visible = true;

    return sprite;
}

Sprite RaceRenderer::CreateEffectSprite(const std::string& effect_type,
                                       const glm::vec2& position, float opacity) const {
    Sprite sprite;
    sprite.texture_id = "particle_smoke";
    sprite.position = position;
    sprite.size = glm::vec2(24.0f, 24.0f);
    sprite.opacity = opacity;
    sprite.layer = static_cast<int>(RenderLayer::Effects);
    sprite.visible = true;

    // Color based on effect type
    if (effect_type == "spark") {
        sprite.color = glm::vec4(1.0f, 1.0f, 0.0f, opacity);  // Yellow
    } else if (effect_type == "dust") {
        sprite.color = glm::vec4(0.8f, 0.7f, 0.5f, opacity);  // Brown
    } else {
        sprite.color = glm::vec4(0.5f, 0.5f, 0.5f, opacity);  // Gray
    }

    return sprite;
}

void RaceRenderer::RenderSpriteLayer(RenderLayer layer) {
    // Filter and render sprites at specific layer
    // This is called during RenderFrame to render layers in order
}

glm::vec3 RaceRenderer::GetVehicleColor(int vehicle_index) const {
    // Assign distinct colors to vehicles
    static const glm::vec3 colors[] = {
        glm::vec3(1.0f, 0.0f, 0.0f),  // Red
        glm::vec3(0.0f, 0.0f, 1.0f),  // Blue
        glm::vec3(0.0f, 1.0f, 0.0f),  // Green
        glm::vec3(1.0f, 1.0f, 0.0f),  // Yellow
        glm::vec3(1.0f, 0.5f, 0.0f),  // Orange
        glm::vec3(1.0f, 0.0f, 1.0f),  // Magenta
        glm::vec3(0.0f, 1.0f, 1.0f),  // Cyan
        glm::vec3(1.0f, 1.0f, 1.0f),  // White
    };

    return colors[vehicle_index % 8];
}

std::string RaceRenderer::GetDebugInfo() const {
    std::stringstream ss;
    ss << "RaceRenderer Debug Info\n";
    ss << "- Initialized: " << (initialized_ ? "Yes" : "No") << "\n";
    ss << "- Assets Loaded: " << (assets_loaded_ ? "Yes" : "No") << "\n";
    ss << "- Loaded Textures: " << loaded_textures_.size() << "\n";
    ss << "- Vehicle Sprites: " << vehicle_render_data_.size() << "\n";
    ss << "- Active Effects: " << active_effects_.size() << "\n";
    ss << "- Track Segments: " << track_render_data_.road_segments.size() << "\n";
    ss << "- Barriers: " << track_render_data_.barriers.size() << "\n";
    ss << "- Decorations: " << track_render_data_.decorations.size() << "\n";
    ss << "- Camera Target: (" << std::fixed << std::setprecision(1)
       << camera_target_.x << ", " << camera_target_.y << ")\n";
    ss << "- Camera Zoom: " << camera_zoom_ << "\n";
    ss << "- Render Track: " << (config_.render_track ? "Yes" : "No") << "\n";
    ss << "- Render Vehicles: " << (config_.render_vehicles ? "Yes" : "No") << "\n";
    ss << "- Render Effects: " << (config_.render_effects ? "Yes" : "No") << "\n";
    return ss.str();
}
