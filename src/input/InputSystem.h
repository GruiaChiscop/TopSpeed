#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>

/**
 * @class InputSystem
 * @brief Centralized input management for keyboard and gamepad
 * 
 * Handles all player input, supports both keyboard and gamepad,
 * with customizable key bindings and input callbacks.
 */
class InputSystem {
public:
    enum class KeyCode {
        // Arrow keys / WASD
        Up,
        Down,
        Left,
        Right,
        W,
        A,
        S,
        D,

        // Numbers
        Num1,
        Num2,
        Num3,
        Num4,
        Num5,

        // Function keys
        F1,
        F2,
        F3,
        F4,
        Escape,
        Space,
        Enter,
        Tab,
        Shift,
        Control,

        // Letters for menus
        P,  // Pause
        R,  // Restart
        Q,  // Quit

        Count
    };

    enum class GamepadButton {
        // D-Pad
        DPadUp,
        DPadDown,
        DPadLeft,
        DPadRight,

        // Face buttons
        A,
        B,
        X,
        Y,

        // Shoulder buttons
        LB,
        RB,
        LT,
        RT,

        // Stick buttons
        LeftStick,
        RightStick,

        // Menu buttons
        Start,
        Back,

        Count
    };

    enum class GamepadAxis {
        LeftStickX,
        LeftStickY,
        RightStickX,
        RightStickY,
        TriggerLeft,   // LT axis
        TriggerRight,  // RT axis

        Count
    };

    enum class InputAction {
        Accelerate,
        Brake,
        SteerLeft,
        SteerRight,
        Pause,
        Restart,
        Quit,
        Horn,
        ShiftUp,
        ShiftDown,
        Handbrake,

        // Menu actions
        Select,
        Back,
        MenuUp,
        MenuDown,

        Count
    };

    struct InputState {
        // Keyboard state
        bool keys[static_cast<int>(KeyCode::Count)] = {false};

        // Gamepad state
        bool gamepad_buttons[static_cast<int>(GamepadButton::Count)] = {false};
        float gamepad_axes[static_cast<int>(GamepadAxis::Count)] = {0.0f};
        bool gamepad_connected = false;

        // Input actions (game-level controls)
        float throttle = 0.0f;      // 0-1
        float brake = 0.0f;         // 0-1
        float steering = 0.0f;      // -1 to 1
        bool horn_pressed = false;
        bool shift_up_pressed = false;
        bool shift_down_pressed = false;
        bool handbrake_pressed = false;
        bool pause_pressed = false;
    };

    using InputCallback = std::function<void(InputAction)>;

    InputSystem();
    ~InputSystem();

    // Initialization
    bool Initialize();
    void Shutdown();

    // Input polling (call once per frame)
    void Update(float deltaTime);

    // Key queries
    bool IsKeyPressed(KeyCode key) const;
    bool IsKeyDown(KeyCode key) const;
    bool IsKeyReleased(KeyCode key) const;

    // Gamepad queries
    bool IsGamepadConnected() const { return current_state_.gamepad_connected; }
    bool IsGamepadButtonPressed(GamepadButton button) const;
    bool IsGamepadButtonDown(GamepadButton button) const;
    bool IsGamepadButtonReleased(GamepadButton button) const;
    float GetGamepadAxis(GamepadAxis axis) const;

    // Action-level input (game controls)
    float GetThrottle() const { return current_state_.throttle; }
    float GetBrake() const { return current_state_.brake; }
    float GetSteering() const { return current_state_.steering; }
    bool IsHornPressed() const { return current_state_.horn_pressed; }
    bool IsShiftUpPressed() const { return current_state_.shift_up_pressed; }
    bool IsShiftDownPressed() const { return current_state_.shift_down_pressed; }
    bool IsHandbrakePressed() const { return current_state_.handbrake_pressed; }
    bool IsPausePressed() const { return current_state_.pause_pressed; }

    // Input mapping
    void MapActionToKey(InputAction action, KeyCode key);
    void MapActionToGamepadButton(InputAction action, GamepadButton button);
    void MapActionToGamepadAxis(InputAction action, GamepadAxis axis);
    void ResetInputMapping();

    // Callbacks
    void RegisterActionCallback(InputAction action, InputCallback callback);
    void UnregisterActionCallback(InputAction action);

    // Analog stick deadzone
    void SetDeadzone(float deadzone) { deadzone_ = std::max(0.0f, std::min(1.0f, deadzone)); }
    float GetDeadzone() const { return deadzone_; }

    // Input state queries
    const InputState& GetCurrentState() const { return current_state_; }
    const InputState& GetPreviousState() const { return previous_state_; }

    // Debug
    std::string GetDebugInfo() const;

private:
    struct KeyBinding {
        KeyCode key = KeyCode::Up;
        GamepadButton gamepad_button = GamepadButton::A;
        GamepadAxis gamepad_axis = GamepadAxis::LeftStickX;
        bool is_axis = false;  // Is this action bound to an axis?
    };

    InputState current_state_;
    InputState previous_state_;
    std::unordered_map<int, KeyBinding> action_bindings_;
    std::unordered_map<int, std::vector<InputCallback>> action_callbacks_;
    float deadzone_ = 0.15f;  // Gamepad deadzone

    // Helper methods
    void PollKeyboard();
    void PollGamepad();
    void ProcessInput();
    void UpdateActionStates();
    void InvokeActionCallbacks(InputAction action);
    void SetupDefaultBindings();
    float ApplyDeadzone(float value) const;
};
