#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

/**
 * @class Sprite
 * @brief Represents a 2D sprite with position, rotation, and scale
 */
struct Sprite {
    std::string texture_id;
    glm::vec2 position;
    glm::vec2 size;
    float rotation = 0.0f;           // Radians
    glm::vec2 scale = glm::vec2(1.0f);
    glm::vec4 color = glm::vec4(1.0f);  // RGBA
    float opacity = 1.0f;
    int layer = 0;                   // Rendering order
    bool visible = true;
    bool flip_horizontal = false;
    bool flip_vertical = false;
};

/**
 * @class RenderSystem
 * @brief Manages sprite rendering, camera, and graphics operations
 * 
 * Handles 2D sprite rendering with layers, camera management,
 * transformations, and viewport handling.
 */
class RenderSystem {
public:
    enum class BlendMode {
        Normal,
        Additive,
        Multiply,
        Screen
    };

    enum class FilterMode {
        Nearest,   // Pixel-perfect, no smoothing
        Linear,    // Smooth interpolation
        Anisotropic  // High-quality filtering
    };

    struct Camera {
        glm::vec2 position = glm::vec2(0.0f);
        float zoom = 1.0f;
        float rotation = 0.0f;
        glm::vec2 target = glm::vec2(0.0f);  // Focus point
        bool follow_target = false;
        float follow_smoothing = 0.1f;
    };

    struct RenderConfig {
        int screen_width = 1920;
        int screen_height = 1080;
        bool vsync_enabled = true;
        FilterMode texture_filter = FilterMode::Linear;
        BlendMode default_blend = BlendMode::Normal;
        bool anti_aliasing = true;
        float background_brightness = 1.0f;
        glm::vec3 background_color = glm::vec3(0.1f, 0.15f, 0.2f);
    };

    RenderSystem();
    ~RenderSystem();

    // Initialization
    bool Initialize(const RenderConfig& config);
    void Shutdown();

    // Frame management
    void BeginFrame();
    void EndFrame();
    void ClearScreen(const glm::vec3& color = glm::vec3(0.0f));
    void Present();  // Display frame

    // Sprite rendering
    void DrawSprite(const Sprite& sprite);
    void DrawSpriteEx(const Sprite& sprite, BlendMode blend_mode);
    void DrawSprites(const std::vector<Sprite>& sprites);

    // Texture management
    bool LoadTexture(const std::string& texture_id, const std::string& file_path);
    void UnloadTexture(const std::string& texture_id);
    bool HasTexture(const std::string& texture_id) const;
    glm::vec2 GetTextureSize(const std::string& texture_id) const;

    // Camera control
    void SetCamera(const Camera& camera) { camera_ = camera; }
    Camera GetCamera() const { return camera_; }
    void UpdateCamera(const glm::vec2& target);
    void SetCameraPosition(const glm::vec2& position);
    void SetCameraZoom(float zoom);
    void SetCameraRotation(float rotation);
    glm::vec2 ScreenToWorldPos(const glm::vec2& screen_pos) const;
    glm::vec2 WorldToScreenPos(const glm::vec2& world_pos) const;

    // Batch rendering (for performance)
    void BeginBatch();
    void EndBatch();
    void FlushBatch();

    // Screen/Viewport
    int GetScreenWidth() const { return config_.screen_width; }
    int GetScreenHeight() const { return config_.screen_height; }
    void SetScreenSize(int width, int height);
    glm::vec2 GetScreenCenter() const {
        return glm::vec2(config_.screen_width * 0.5f, config_.screen_height * 0.5f);
    }

    // Configuration
    void SetBackgroundColor(const glm::vec3& color);
    void SetBlendMode(BlendMode mode);
    void SetTextureFilter(FilterMode filter);
    RenderConfig GetConfig() const { return config_; }

    // Debug/Info
    std::string GetDebugInfo() const;
    int GetLoadedTextureCount() const { return static_cast<int>(textures_.size()); }
    int GetBatchSize() const { return static_cast<int>(batch_sprites_.size()); }

private:
    struct TextureData {
        std::string file_path;
        glm::vec2 size;
        unsigned int opengl_id = 0;
        bool loaded = false;
    };

    RenderConfig config_;
    Camera camera_;
    std::unordered_map<std::string, TextureData> textures_;
    std::vector<Sprite> batch_sprites_;
    bool batching_enabled_ = false;
    bool frame_started_ = false;
    bool initialized_ = false;

    // Helper methods
    void RenderBatch();
    void SortSpritesByLayer(std::vector<Sprite>& sprites) const;
    glm::mat4 GetViewProjectionMatrix() const;
    glm::mat4 GetSpriteTransformMatrix(const Sprite& sprite) const;
};
