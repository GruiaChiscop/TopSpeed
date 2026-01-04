#include "RenderSystem.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

RenderSystem::RenderSystem() {
    camera_.position = glm::vec2(0.0f);
    camera_.zoom = 1.0f;
}

RenderSystem::~RenderSystem() {
    if (initialized_) {
        Shutdown();
    }
}

bool RenderSystem::Initialize(const RenderConfig& config) {
    if (initialized_) {
        return true;
    }

    config_ = config;
    camera_.position = glm::vec2(config.screen_width * 0.5f, config.screen_height * 0.5f);

    // Initialize graphics context would happen here
    // For now, we're setting up the framework

    initialized_ = true;
    return true;
}

void RenderSystem::Shutdown() {
    for (auto& texture : textures_) {
        UnloadTexture(texture.first);
    }
    textures_.clear();
    batch_sprites_.clear();
    initialized_ = false;
}

void RenderSystem::BeginFrame() {
    frame_started_ = true;
    ClearScreen(config_.background_color);
}

void RenderSystem::EndFrame() {
    FlushBatch();
    frame_started_ = false;
}

void RenderSystem::ClearScreen(const glm::vec3& color) {
    // In a real implementation, this would clear the framebuffer
    // For now, it's a no-op
}

void RenderSystem::Present() {
    // In a real implementation, this would swap buffers
    // For now, it's a no-op
}

void RenderSystem::DrawSprite(const Sprite& sprite) {
    DrawSpriteEx(sprite, config_.default_blend);
}

void RenderSystem::DrawSpriteEx(const Sprite& sprite, BlendMode blend_mode) {
    if (!sprite.visible || !HasTexture(sprite.texture_id)) {
        return;
    }

    if (batching_enabled_) {
        batch_sprites_.push_back(sprite);
    } else {
        // Immediate rendering
        // In a real implementation, this would render directly
    }
}

void RenderSystem::DrawSprites(const std::vector<Sprite>& sprites) {
    for (const auto& sprite : sprites) {
        DrawSprite(sprite);
    }
}

bool RenderSystem::LoadTexture(const std::string& texture_id, const std::string& file_path) {
    if (textures_.find(texture_id) != textures_.end()) {
        return true;  // Already loaded
    }

    TextureData texture_data;
    texture_data.file_path = file_path;
    texture_data.loaded = true;
    texture_data.size = glm::vec2(64.0f, 64.0f);  // Default size

    // In a real implementation, we would load from disk here
    // For now, we're just registering the texture

    textures_[texture_id] = texture_data;
    return true;
}

void RenderSystem::UnloadTexture(const std::string& texture_id) {
    auto it = textures_.find(texture_id);
    if (it != textures_.end()) {
        textures_.erase(it);
    }
}

bool RenderSystem::HasTexture(const std::string& texture_id) const {
    return textures_.find(texture_id) != textures_.end();
}

glm::vec2 RenderSystem::GetTextureSize(const std::string& texture_id) const {
    auto it = textures_.find(texture_id);
    if (it != textures_.end()) {
        return it->second.size;
    }
    return glm::vec2(0.0f);
}

void RenderSystem::UpdateCamera(const glm::vec2& target) {
    if (!camera_.follow_target) {
        return;
    }

    // Smooth camera movement
    glm::vec2 direction = target - camera_.target;
    float distance = glm::length(direction);

    if (distance > 0.1f) {
        glm::vec2 move_direction = glm::normalize(direction);
        float move_speed = distance * camera_.follow_smoothing;
        camera_.position += move_direction * move_speed;
        camera_.target = target;
    }
}

void RenderSystem::SetCameraPosition(const glm::vec2& position) {
    camera_.position = position;
}

void RenderSystem::SetCameraZoom(float zoom) {
    camera_.zoom = std::max(0.1f, zoom);
}

void RenderSystem::SetCameraRotation(float rotation) {
    camera_.rotation = rotation;
}

glm::vec2 RenderSystem::ScreenToWorldPos(const glm::vec2& screen_pos) const {
    // Convert screen coordinates to world coordinates
    glm::vec2 center = GetScreenCenter();
    glm::vec2 relative_pos = screen_pos - center;
    relative_pos /= camera_.zoom;
    return camera_.position + relative_pos;
}

glm::vec2 RenderSystem::WorldToScreenPos(const glm::vec2& world_pos) const {
    // Convert world coordinates to screen coordinates
    glm::vec2 relative_pos = world_pos - camera_.position;
    relative_pos *= camera_.zoom;
    return GetScreenCenter() + relative_pos;
}

void RenderSystem::BeginBatch() {
    batching_enabled_ = true;
    batch_sprites_.clear();
}

void RenderSystem::EndBatch() {
    FlushBatch();
    batching_enabled_ = false;
}

void RenderSystem::FlushBatch() {
    if (batch_sprites_.empty()) {
        return;
    }

    // Sort sprites by layer
    SortSpritesByLayer(batch_sprites_);

    // Render all sprites
    for (const auto& sprite : batch_sprites_) {
        // In a real implementation, actual rendering would happen here
    }

    batch_sprites_.clear();
}

void RenderSystem::SetScreenSize(int width, int height) {
    config_.screen_width = width;
    config_.screen_height = height;
}

void RenderSystem::SetBackgroundColor(const glm::vec3& color) {
    config_.background_color = color;
}

void RenderSystem::SetBlendMode(BlendMode mode) {
    config_.default_blend = mode;
}

void RenderSystem::SetTextureFilter(FilterMode filter) {
    config_.texture_filter = filter;
}

void RenderSystem::RenderBatch() {
    // Sort by layer and render
    SortSpritesByLayer(batch_sprites_);

    for (const auto& sprite : batch_sprites_) {
        // Render sprite using its transform matrix
        glm::mat4 transform = GetSpriteTransformMatrix(sprite);
        // In a real implementation, this would set up shader uniforms
        // and render the sprite quad
    }
}

void RenderSystem::SortSpritesByLayer(std::vector<Sprite>& sprites) const {
    std::sort(sprites.begin(), sprites.end(),
              [](const Sprite& a, const Sprite& b) {
                  return a.layer < b.layer;
              });
}

glm::mat4 RenderSystem::GetViewProjectionMatrix() const {
    // Create orthographic projection matrix
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(config_.screen_width),
                                      static_cast<float>(config_.screen_height), 0.0f,
                                      -1.0f, 1.0f);

    // Create view matrix with camera position and zoom
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(-camera_.position.x, -camera_.position.y, 0.0f));
    view = glm::scale(view, glm::vec3(camera_.zoom, camera_.zoom, 1.0f));
    view = glm::rotate(view, camera_.rotation, glm::vec3(0.0f, 0.0f, 1.0f));

    return projection * view;
}

glm::mat4 RenderSystem::GetSpriteTransformMatrix(const Sprite& sprite) const {
    glm::mat4 transform = glm::mat4(1.0f);

    // Translate
    transform = glm::translate(transform, glm::vec3(sprite.position.x, sprite.position.y, sprite.layer * 0.001f));

    // Rotate
    transform = glm::rotate(transform, sprite.rotation, glm::vec3(0.0f, 0.0f, 1.0f));

    // Scale
    transform = glm::scale(transform, glm::vec3(sprite.scale.x * sprite.size.x,
                                               sprite.scale.y * sprite.size.y, 1.0f));

    return transform;
}

std::string RenderSystem::GetDebugInfo() const {
    std::stringstream ss;
    ss << "RenderSystem Debug Info\n";
    ss << "- Initialized: " << (initialized_ ? "Yes" : "No") << "\n";
    ss << "- Screen: " << config_.screen_width << "x" << config_.screen_height << "\n";
    ss << "- Camera Position: (" << std::fixed << std::setprecision(1)
       << camera_.position.x << ", " << camera_.position.y << ")\n";
    ss << "- Camera Zoom: " << camera_.zoom << "\n";
    ss << "- Camera Rotation: " << camera_.rotation << " rad\n";
    ss << "- Loaded Textures: " << textures_.size() << "\n";
    ss << "- Batch Size: " << batch_sprites_.size() << "\n";
    ss << "- Batching: " << (batching_enabled_ ? "Yes" : "No") << "\n";
    ss << "- Frame Started: " << (frame_started_ ? "Yes" : "No") << "\n";
    ss << "- Background Color: (" << std::setprecision(2)
       << config_.background_color.r << ", "
       << config_.background_color.g << ", "
       << config_.background_color.b << ")\n";
    ss << "- VSync: " << (config_.vsync_enabled ? "Enabled" : "Disabled") << "\n";
    return ss.str();
}
