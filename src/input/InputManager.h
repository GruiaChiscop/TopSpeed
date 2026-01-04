#pragma once

#include <SDL3/SDL.h>
#include <map>
#include <functional>
#include <cstdint>

namespace TopSpeed {

/**
 * Input manager - SDL3 abstraction layer
 * 
 * Pattern inspired by NVGT's SDL2 input migration:
 * - Unified keyboard, mouse, gamepad interface
 * - Configurable key bindings
 * - Analog axis support (joystick/controller)
 * - Non-blocking event polling
 * - Callback support for key events
 * 
 * Usage:
 *   InputManager input;
 *   input.Initialize();
 *   input.MapKey("accelerate", SDL_SCANCODE_UP);
 *   input.Update();
 *   
 *   if (input.IsKeyPressed("accelerate")) {
 *       // Handle input
 *   }
 */
class InputManager {
public:
    /**
     * Key state change callback
     * @param key_name Bind name (e.g., "accelerate")
     * @param pressed True if pressed, false if released
     */
    using KeyCallback = std::function<void(const std::string& key_name, bool pressed)>;
    
    /**
     * Axis change callback (for analog controls)
     * @param axis_name Axis name (e.g., "steering")
     * @param value Axis value (-1.0 to 1.0)
     */
    using AxisCallback = std::function<void(const std::string& axis_name, float value)>;
    
    InputManager();
    ~InputManager();
    
    /**
     * Initialize input system
     * @return True if successful
     */
    bool Initialize();
    
    /**
     * Update input state (call once per frame)
     * Polls SDL events and updates key/axis states
     */
    void Update();
    
    /**
     * Shut down input system
     */
    void Shutdown();
    
    // Key binding management
    /**
     * Map a semantic action to a physical key
     * @param action Action name (e.g., "accelerate", "brake")
     * @param scancode SDL_SCANCODE_* value
     */
    void MapKey(const std::string& action, SDL_Scancode scancode);
    
    /**
     * Map analog axis (joystick)
     * @param action Action name (e.g., "steering")
     * @param joystick_index Joystick device index
     * @param axis Joystick axis index
     * @param invert Whether to invert axis
     */
    void MapAxis(const std::string& action, int joystick_index, int axis, bool invert = false);
    
    /**
     * Get scancode mapped to action
     */
    SDL_Scancode GetMappedKey(const std::string& action) const;
    
    /**
     * Clear all key bindings
     */
    void ClearKeyBindings();
    
    // Key state queries
    /**
     * Check if key is currently held down
     * @param action Action name (e.g., "accelerate")
     */
    bool IsKeyDown(const std::string& action) const;
    
    /**
     * Check if key was just pressed this frame
     */
    bool IsKeyPressed(const std::string& action) const;
    
    /**
     * Check if key was just released this frame
     */
    bool IsKeyReleased(const std::string& action) const;
    
    /**
     * Check by raw scancode (for unmapped keys)
     */
    bool IsScancodeDown(SDL_Scancode scancode) const;
    bool IsScancodePressed(SDL_Scancode scancode) const;
    
    // Axis queries
    /**
     * Get analog axis value
     * @param action Action name
     * @return Value from -1.0 to 1.0
     */
    float GetAxisValue(const std::string& action) const;
    
    /**
     * Get axis value with dead zone applied
     * @param action Action name
     * @param dead_zone Dead zone threshold (0.0-0.5)
     */
    float GetAxisValueDeadZone(const std::string& action, float dead_zone) const;
    
    /**
     * Set joystick dead zone (applies to all axes)
     * @param dead_zone Dead zone threshold (0.0-0.5, default 0.15)
     */
    void SetDeadZone(float dead_zone) { dead_zone_ = dead_zone; }
    float GetDeadZone() const { return dead_zone_; }
    
    // Mouse support
    /**
     * Get mouse X position
     */
    int GetMouseX() const { return mouse_x_; }
    
    /**
     * Get mouse Y position
     */
    int GetMouseY() const { return mouse_y_; }
    
    /**
     * Get relative mouse movement (for FPS-style controls)
     */
    int GetMouseDeltaX() const { return mouse_delta_x_; }
    int GetMouseDeltaY() const { return mouse_delta_y_; }
    
    /**
     * Check if mouse button is down
     * @param button SDL_BUTTON_LEFT, SDL_BUTTON_MIDDLE, SDL_BUTTON_RIGHT
     */
    bool IsMouseButtonDown(uint8_t button) const;
    bool IsMouseButtonPressed(uint8_t button) const;
    
    /**
     * Show/hide mouse cursor
     */
    void SetMouseVisible(bool visible);
    bool IsMouseVisible() const { return mouse_visible_; }
    
    /**
     * Capture mouse (confine to window)
     */
    void SetMouseCapture(bool captured);
    bool IsMouseCaptured() const { return mouse_captured_; }
    
    // Text input
    /**
     * Start text input mode (shows virtual keyboard on mobile)
     */
    void StartTextInput();
    
    /**
     * Stop text input mode
     */
    void StopTextInput();
    
    /**
     * Get text input from current frame
     */
    const std::string& GetTextInput() const { return text_input_; }
    
    // Event callbacks
    /**
     * Register callback for key state changes
     */
    void SetKeyCallback(KeyCallback callback) { key_callback_ = callback; }
    
    /**
     * Register callback for axis changes
     */
    void SetAxisCallback(AxisCallback callback) { axis_callback_ = callback; }
    
    // Controller support
    /**
     * Check if gamepad is connected
     * @param index Joystick index
     */
    bool IsGamepadConnected(int index) const;
    
    /**
     * Get number of connected gamepads
     */
    int GetGamepadCount() const { return gamepad_count_; }
    
    /**
     * Vibrate gamepad (rumble)
     * @param index Gamepad index
     * @param left Left motor strength (0.0-1.0)
     * @param right Right motor strength (0.0-1.0)
     * @param duration_ms Duration in milliseconds
     */
    void VibrationFeedback(int index, float left, float right, uint32_t duration_ms);
    
    // Debug
    /**
     * Get key binding info for debugging
     */
    std::string GetDebugInfo() const;
    
    /**
     * Print current key states
     */
    void PrintKeyStates() const;

private:
    // Key binding mapping
    struct KeyBinding {
        SDL_Scancode scancode;
    };
    
    struct AxisBinding {
        int joystick_index;
        int axis;
        bool inverted;
    };
    
    std::map<std::string, KeyBinding> key_bindings_;
    std::map<std::string, AxisBinding> axis_bindings_;
    
    // Key state tracking (current frame)
    std::map<SDL_Scancode, bool> keys_down_;
    std::map<SDL_Scancode, bool> keys_pressed_;   // Just pressed this frame
    std::map<SDL_Scancode, bool> keys_released_;  // Just released this frame
    
    // Axis state
    std::map<std::string, float> axis_values_;
    
    // Mouse
    int mouse_x_ = 0;
    int mouse_y_ = 0;
    int mouse_delta_x_ = 0;
    int mouse_delta_y_ = 0;
    uint8_t mouse_buttons_ = 0;
    uint8_t mouse_buttons_pressed_ = 0;
    uint8_t mouse_buttons_released_ = 0;
    bool mouse_visible_ = true;
    bool mouse_captured_ = false;
    
    // Text input
    std::string text_input_;
    
    // Gamepads
    int gamepad_count_ = 0;
    std::map<int, SDL_Gamepad*> gamepads_;
    
    // Settings
    float dead_zone_ = 0.15f;
    
    // Callbacks
    KeyCallback key_callback_;
    AxisCallback axis_callback_;
    
    // Internal helpers
    void HandleEvent(const SDL_Event& event);
    void UpdateKeyStates();
    void UpdateAxisStates();
    float ApplyDeadZone(float value, float threshold);
};

} // namespace TopSpeed
