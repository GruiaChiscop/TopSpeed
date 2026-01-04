#include "InputSystem.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

InputSystem::InputSystem() {
    SetupDefaultBindings();
}

InputSystem::~InputSystem() {
    Shutdown();
}

bool InputSystem::Initialize() {
    SetupDefaultBindings();
    return true;
}

void InputSystem::Shutdown() {
    action_callbacks_.clear();
}

void InputSystem::Update(float deltaTime) {
    // Save previous state
    previous_state_ = current_state_;

    // Poll input devices
    PollKeyboard();
    PollGamepad();

    // Process raw input into action states
    ProcessInput();
    UpdateActionStates();
}

void InputSystem::PollKeyboard() {
    // In a real implementation, this would poll the OS for key states
    // For now, we use a mock implementation
    // Actual implementation would use SDL2, GLFW, or Windows API
}

void InputSystem::PollGamepad() {
    // In a real implementation, this would poll gamepad input
    // For now, we use a mock implementation
    // Actual implementation would use SDL2 or XInput
}

void InputSystem::ProcessInput() {
    // Convert raw input to throttle/brake/steering
    // This reads from current_state_.keys and gamepad_buttons/axes
    // and produces throttle, brake, steering outputs
}

void InputSystem::UpdateActionStates() {
    // Calculate analog values from input
    current_state_.throttle = GetGamepadAxis(GamepadAxis::TriggerRight);
    if (current_state_.throttle < 0.05f) {
        // Check keyboard accelerate
        if (IsKeyDown(KeyCode::W) || IsKeyDown(KeyCode::Up)) {
            current_state_.throttle = 1.0f;
        } else {
            current_state_.throttle = 0.0f;
        }
    }

    current_state_.brake = GetGamepadAxis(GamepadAxis::TriggerLeft);
    if (current_state_.brake < 0.05f) {
        // Check keyboard brake
        if (IsKeyDown(KeyCode::S) || IsKeyDown(KeyCode::Down)) {
            current_state_.brake = 1.0f;
        } else {
            current_state_.brake = 0.0f;
        }
    }

    // Steering
    float gamepad_steering = GetGamepadAxis(GamepadAxis::LeftStickX);
    gamepad_steering = ApplyDeadzone(gamepad_steering);

    current_state_.steering = gamepad_steering;
    if (std::abs(gamepad_steering) < 0.1f) {
        // Check keyboard steering
        float keyboard_steering = 0.0f;
        if (IsKeyDown(KeyCode::A) || IsKeyDown(KeyCode::Left)) {
            keyboard_steering = -1.0f;
        }
        if (IsKeyDown(KeyCode::D) || IsKeyDown(KeyCode::Right)) {
            keyboard_steering = 1.0f;
        }
        current_state_.steering = keyboard_steering;
    }

    // Horn
    current_state_.horn_pressed = IsKeyPressed(KeyCode::Space);

    // Transmission
    current_state_.shift_up_pressed = IsKeyPressed(KeyCode::W);
    current_state_.shift_down_pressed = IsKeyPressed(KeyCode::S);

    // Handbrake
    current_state_.handbrake_pressed = IsKeyDown(KeyCode::Shift) || IsGamepadButtonDown(GamepadButton::RB);

    // Pause
    current_state_.pause_pressed = IsKeyPressed(KeyCode::Escape) || IsKeyPressed(KeyCode::P);
}

bool InputSystem::IsKeyPressed(KeyCode key) const {
    int key_idx = static_cast<int>(key);
    return current_state_.keys[key_idx] && !previous_state_.keys[key_idx];
}

bool InputSystem::IsKeyDown(KeyCode key) const {
    int key_idx = static_cast<int>(key);
    return current_state_.keys[key_idx];
}

bool InputSystem::IsKeyReleased(KeyCode key) const {
    int key_idx = static_cast<int>(key);
    return !current_state_.keys[key_idx] && previous_state_.keys[key_idx];
}

bool InputSystem::IsGamepadButtonPressed(GamepadButton button) const {
    int btn_idx = static_cast<int>(button);
    return current_state_.gamepad_buttons[btn_idx] && !previous_state_.gamepad_buttons[btn_idx];
}

bool InputSystem::IsGamepadButtonDown(GamepadButton button) const {
    int btn_idx = static_cast<int>(button);
    return current_state_.gamepad_buttons[btn_idx];
}

bool InputSystem::IsGamepadButtonReleased(GamepadButton button) const {
    int btn_idx = static_cast<int>(button);
    return !current_state_.gamepad_buttons[btn_idx] && previous_state_.gamepad_buttons[btn_idx];
}

float InputSystem::GetGamepadAxis(GamepadAxis axis) const {
    int axis_idx = static_cast<int>(axis);
    if (axis_idx >= 0 && axis_idx < static_cast<int>(GamepadAxis::Count)) {
        return current_state_.gamepad_axes[axis_idx];
    }
    return 0.0f;
}

void InputSystem::MapActionToKey(InputAction action, KeyCode key) {
    int action_idx = static_cast<int>(action);
    if (action_idx >= 0 && action_idx < static_cast<int>(InputAction::Count)) {
        action_bindings_[action_idx].key = key;
    }
}

void InputSystem::MapActionToGamepadButton(InputAction action, GamepadButton button) {
    int action_idx = static_cast<int>(action);
    if (action_idx >= 0 && action_idx < static_cast<int>(InputAction::Count)) {
        action_bindings_[action_idx].gamepad_button = button;
    }
}

void InputSystem::MapActionToGamepadAxis(InputAction action, GamepadAxis axis) {
    int action_idx = static_cast<int>(action);
    if (action_idx >= 0 && action_idx < static_cast<int>(InputAction::Count)) {
        action_bindings_[action_idx].gamepad_axis = axis;
        action_bindings_[action_idx].is_axis = true;
    }
}

void InputSystem::ResetInputMapping() {
    SetupDefaultBindings();
}

void InputSystem::RegisterActionCallback(InputAction action, InputCallback callback) {
    int action_idx = static_cast<int>(action);
    action_callbacks_[action_idx].push_back(callback);
}

void InputSystem::UnregisterActionCallback(InputAction action) {
    int action_idx = static_cast<int>(action);
    action_callbacks_.erase(action_idx);
}

void InputSystem::InvokeActionCallbacks(InputAction action) {
    int action_idx = static_cast<int>(action);
    auto it = action_callbacks_.find(action_idx);
    if (it != action_callbacks_.end()) {
        for (const auto& callback : it->second) {
            callback(action);
        }
    }
}

float InputSystem::ApplyDeadzone(float value) const {
    if (std::abs(value) < deadzone_) {
        return 0.0f;
    }
    // Scale the value to remove deadzone area
    float sign = (value >= 0.0f) ? 1.0f : -1.0f;
    return sign * ((std::abs(value) - deadzone_) / (1.0f - deadzone_));
}

void InputSystem::SetupDefaultBindings() {
    // Default racing input bindings
    MapActionToKey(InputAction::Accelerate, KeyCode::W);
    MapActionToKey(InputAction::Brake, KeyCode::S);
    MapActionToKey(InputAction::SteerLeft, KeyCode::A);
    MapActionToKey(InputAction::SteerRight, KeyCode::D);
    MapActionToKey(InputAction::Pause, KeyCode::Escape);
    MapActionToKey(InputAction::Restart, KeyCode::R);
    MapActionToKey(InputAction::Quit, KeyCode::Q);
    MapActionToKey(InputAction::Horn, KeyCode::Space);
    MapActionToKey(InputAction::ShiftUp, KeyCode::W);
    MapActionToKey(InputAction::ShiftDown, KeyCode::S);
    MapActionToKey(InputAction::Handbrake, KeyCode::Shift);

    // Default gamepad bindings
    MapActionToGamepadAxis(InputAction::Accelerate, GamepadAxis::TriggerRight);
    MapActionToGamepadAxis(InputAction::Brake, GamepadAxis::TriggerLeft);
    MapActionToGamepadAxis(InputAction::SteerLeft, GamepadAxis::LeftStickX);
    MapActionToGamepadButton(InputAction::Horn, GamepadButton::A);
    MapActionToGamepadButton(InputAction::ShiftUp, GamepadButton::Y);
    MapActionToGamepadButton(InputAction::ShiftDown, GamepadButton::X);
    MapActionToGamepadButton(InputAction::Handbrake, GamepadButton::RB);
    MapActionToGamepadButton(InputAction::Pause, GamepadButton::Start);
}

std::string InputSystem::GetDebugInfo() const {
    std::stringstream ss;
    ss << "InputSystem Debug Info\n";
    ss << "- Gamepad Connected: " << (current_state_.gamepad_connected ? "Yes" : "No") << "\n";
    ss << "- Throttle: " << std::fixed << std::setprecision(2) << current_state_.throttle << "\n";
    ss << "- Brake: " << current_state_.brake << "\n";
    ss << "- Steering: " << current_state_.steering << "\n";
    ss << "- Handbrake: " << (current_state_.handbrake_pressed ? "Pressed" : "Released") << "\n";
    ss << "- Pause: " << (current_state_.pause_pressed ? "Pressed" : "Released") << "\n";
    ss << "- Horn: " << (current_state_.horn_pressed ? "Pressed" : "Released") << "\n";
    ss << "- Deadzone: " << deadzone_ << "\n";
    return ss.str();
}
