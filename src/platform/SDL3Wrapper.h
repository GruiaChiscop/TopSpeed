#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <vector>
#include <memory>

namespace TopSpeed {

/**
 * SDL3Wrapper provides a modern replacement for DirectX functionality
 * using SDL3 for window management, rendering, and input handling.
 */
class SDL3Wrapper {
public:
    SDL3Wrapper();
    ~SDL3Wrapper();

    /**
     * Initialize SDL3 subsystems
     * @return true if initialization successful
     */
    bool Initialize();

    /**
     * Create game window
     * @param width Window width in pixels
     * @param height Window height in pixels
     * @param title Window title
     * @return true if window created successfully
     */
    bool CreateWindow(int width, int height, const std::string& title);

    /**
     * Process all pending events
     * @return true if application should continue, false if quit requested
     */
    bool ProcessEvents();

    /**
     * Get keyboard state for current frame
     * @return const pointer to keyboard state array
     */
    const bool* GetKeyboardState() const;

    /**
     * Check if a specific key is pressed
     * @param scancode SDL_Scancode for the key
     * @return true if key is currently pressed
     */
    bool IsKeyPressed(SDL_Scancode scancode) const;

    /**
     * Clear the screen
     * @param r Red component (0-255)
     * @param g Green component (0-255)
     * @param b Blue component (0-255)
     */
    void ClearScreen(uint8_t r, uint8_t g, uint8_t b);

    /**
     * Present the rendered frame
     */
    void Present();

    /**
     * Get window dimensions
     * @return width, height pair
     */
    std::pair<int, int> GetWindowSize() const;

    /**
     * Set window size
     */
    void SetWindowSize(int width, int height);

    /**
     * Shutdown SDL3 and cleanup resources
     */
    void Shutdown();

    /**
     * Get underlying SDL_Window pointer (for advanced usage)
     */
    SDL_Window* GetNativeWindow() { return window_; }

    /**
     * Get underlying SDL_Renderer pointer (for advanced usage)
     */
    SDL_Renderer* GetNativeRenderer() { return renderer_; }

private:
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    int window_width_;
    int window_height_;
    bool should_quit_;
    std::vector<bool> keyboard_state_;
};

} // namespace TopSpeed
