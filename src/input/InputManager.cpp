#include "InputManager.h"
#include <iostream>
#include <algorithm>
#include <sstream>

namespace TopSpeed {

InputManager::InputManager() : gamepad_count_(0) {}

InputManager::~InputManager() {
    Shutdown();
}

bool InputManager::Initialize() {
    std::cout << "Initializing InputManager with SDL3..." << std::endl;
    
    // SDL3 is initialized by the audio engine, but we can verify
    // Check if SDL is available
    if (!SDL_WasInit(SDL_INIT_EVENTS)) {
        std::cerr << "Warning: SDL3 events not initialized" << std::endl;
        return false;
    }
    
    // Initialize gamepad support
    std::cout << "\u2713 InputManager initialized" << std::endl;
    return true;
}

void InputManager::Update() {
    // Clear per-frame key states
    keys_pressed_.clear();
    keys_released_.clear();
    
    mouse_delta_x_ = 0;
    mouse_delta_y_ = 0;
    mouse_buttons_pressed_ = 0;
    mouse_buttons_released_ = 0;
    text_input_.clear();
    
    // Poll SDL events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        HandleEvent(event);
    }
    
    // Update gamepad states
    UpdateAxisStates();
}

void InputManager::Shutdown() {
    std::cout << "Shutting down InputManager..." << std::endl;
    
    // Close all gamepads
    for (auto& [index, gamepad] : gamepads_) {
        if (gamepad) {
            SDL_CloseGamepad(gamepad);
        }
    }
    gamepads_.clear();
}

void InputManager::MapKey(const std::string& action, SDL_Scancode scancode) {
    key_bindings_[action] = {scancode};
}

void InputManager::MapAxis(const std::string& action, int joystick_index, int axis, bool invert) {
    axis_bindings_[action] = {joystick_index, axis, invert};
    axis_values_[action] = 0.0f;
}

SDL_Scancode InputManager::GetMappedKey(const std::string& action) const {
    auto it = key_bindings_.find(action);
    if (it != key_bindings_.end()) {
        return it->second.scancode;
    }
    return SDL_SCANCODE_UNKNOWN;
}

void InputManager::ClearKeyBindings() {
    key_bindings_.clear();
}

bool InputManager::IsKeyDown(const std::string& action) const {
    auto it = key_bindings_.find(action);
    if (it != key_bindings_.end()) {
        return IsScancodeDown(it->second.scancode);
    }
    return false;
}

bool InputManager::IsKeyPressed(const std::string& action) const {
    auto it = key_bindings_.find(action);
    if (it != key_bindings_.end()) {
        return IsScancodePressed(it->second.scancode);
    }
    return false;
}

bool InputManager::IsKeyReleased(const std::string& action) const {
    auto it = key_bindings_.find(action);
    if (it != key_bindings_.end()) {
        auto scancode = it->second.scancode;
        return keys_released_.find(scancode) != keys_released_.end();
    }
    return false;
}

bool InputManager::IsScancodeDown(SDL_Scancode scancode) const {
    auto it = keys_down_.find(scancode);
    return it != keys_down_.end() && it->second;
}

bool InputManager::IsScancodePressed(SDL_Scancode scancode) const {
    auto it = keys_pressed_.find(scancode);
    return it != keys_pressed_.end() && it->second;
}

float InputManager::GetAxisValue(const std::string& action) const {
    auto it = axis_values_.find(action);
    if (it != axis_values_.end()) {
        return it->second;
    }
    return 0.0f;
}

float InputManager::GetAxisValueDeadZone(const std::string& action, float dead_zone) const {
    float value = GetAxisValue(action);
    return ApplyDeadZone(value, dead_zone);
}

float InputManager::ApplyDeadZone(float value, float threshold) {
    threshold = std::clamp(threshold, 0.0f, 0.5f);
    
    if (std::abs(value) < threshold) {
        return 0.0f;
    }
    
    // Rescale to remove dead zone
    if (value > 0) {
        return (value - threshold) / (1.0f - threshold);
    } else {
        return (value + threshold) / (1.0f - threshold);
    }
}

bool InputManager::IsMouseButtonDown(uint8_t button) const {
    return (mouse_buttons_ & SDL_BUTTON(button)) != 0;
}

bool InputManager::IsMouseButtonPressed(uint8_t button) const {
    return (mouse_buttons_pressed_ & SDL_BUTTON(button)) != 0;
}

void InputManager::SetMouseVisible(bool visible) {
    mouse_visible_ = visible;
    SDL_ShowCursor(visible);
}

void InputManager::SetMouseCapture(bool captured) {
    mouse_captured_ = captured;
    // SDL3: Use SDL_SetWindowMouseGrab instead of SDL_SetRelativeMouseMode
    // This would require window pointer, so we'll note it for future
}

void InputManager::StartTextInput() {
    SDL_StartTextInput();
}

void InputManager::StopTextInput() {
    SDL_StopTextInput();
}

bool InputManager::IsGamepadConnected(int index) const {
    return gamepads_.find(index) != gamepads_.end();
}

void InputManager::VibrationFeedback(int index, float left, float right, uint32_t duration_ms) {
    auto it = gamepads_.find(index);
    if (it != gamepads_.end() && it->second) {
        // SDL_RumbleGamepad for rumble support
        // Requires SDL3.4+, might not be available in earlier versions
        // Would implement: SDL_RumbleGamepad(it->second, left, right, duration_ms);
    }
}

std::string InputManager::GetDebugInfo() const {
    std::ostringstream oss;
    oss << "=== Input Manager ===\n";
    oss << "Key Bindings: " << key_bindings_.size() << "\n";
    for (const auto& [action, binding] : key_bindings_) {
        oss << "  " << action << " -> Scancode " << static_cast<int>(binding.scancode) << "\n";
    }
    oss << "\nAxis Bindings: " << axis_bindings_.size() << "\n";
    for (const auto& [action, binding] : axis_bindings_) {
        oss << "  " << action << " -> Joystick " << binding.joystick_index
            << " Axis " << binding.axis << "\n";
    }
    oss << "\nMouse: (" << mouse_x_ << ", " << mouse_y_ << ")\n";
    oss << "Dead Zone: " << dead_zone_ << "\n";
    oss << "Gamepads: " << gamepad_count_;
    return oss.str();
}

void InputManager::PrintKeyStates() const {
    std::cout << "Active keys:\n";
    for (const auto& [scancode, pressed] : keys_down_) {
        if (pressed) {
            std::cout << "  Scancode " << static_cast<int>(scancode) << "\n";
        }
    }
}

// Private methods

void InputManager::HandleEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_EVENT_KEY_DOWN:
            keys_down_[event.key.scancode] = true;
            keys_pressed_[event.key.scancode] = true;
            
            if (key_callback_) {
                // Find action for this scancode
                for (const auto& [action, binding] : key_bindings_) {
                    if (binding.scancode == event.key.scancode) {
                        key_callback_(action, true);
                        break;
                    }
                }
            }
            break;
            
        case SDL_EVENT_KEY_UP:
            keys_down_[event.key.scancode] = false;
            keys_released_[event.key.scancode] = true;
            
            if (key_callback_) {
                for (const auto& [action, binding] : key_bindings_) {
                    if (binding.scancode == event.key.scancode) {
                        key_callback_(action, false);
                        break;
                    }
                }
            }
            break;
            
        case SDL_EVENT_MOUSE_MOTION:
            mouse_delta_x_ = event.motion.xrel;
            mouse_delta_y_ = event.motion.yrel;
            mouse_x_ = event.motion.x;
            mouse_y_ = event.motion.y;
            break;
            
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            mouse_buttons_ |= SDL_BUTTON(event.button.button);
            mouse_buttons_pressed_ |= SDL_BUTTON(event.button.button);
            break;
            
        case SDL_EVENT_MOUSE_BUTTON_UP:
            mouse_buttons_ &= ~SDL_BUTTON(event.button.button);
            mouse_buttons_released_ |= SDL_BUTTON(event.button.button);
            break;
            
        case SDL_EVENT_TEXT_INPUT:
            text_input_ = event.text.text;
            break;
            
        case SDL_EVENT_GAMEPAD_ADDED:
            // Handle gamepad connection
            gamepad_count_++;
            break;
            
        case SDL_EVENT_GAMEPAD_REMOVED:
            // Handle gamepad disconnection
            gamepad_count_--;
            break;
            
        default:
            break;
    }
}

void InputManager::UpdateKeyStates() {
    // Key states are updated in HandleEvent
}

void InputManager::UpdateAxisStates() {
    // Update axis values from all bound axes
    for (auto& [action, binding] : axis_bindings_) {
        // For now, axis values remain constant within a frame
        // In a full implementation, would query gamepad axes here
        // using SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_*)
        
        if (axis_callback_) {
            axis_callback_(action, axis_values_[action]);
        }
    }
}

} // namespace TopSpeed
