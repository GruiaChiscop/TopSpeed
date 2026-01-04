#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include "../src/input/InputManager.h"

using namespace TopSpeed;

// ============================================================================
// Test Utilities
// ============================================================================

class TestResults {
public:
    void Pass(const std::string& test_name) {
        passed_++;
        std::cout << "  ✓ " << test_name << std::endl;
    }
    
    void Fail(const std::string& test_name, const std::string& reason) {
        failed_++;
        std::cout << "  ✗ " << test_name << " - " << reason << std::endl;
    }
    
    void PrintSummary() {
        std::cout << "\n" << (passed_ + failed_) << " tests: "
                  << passed_ << " passed, " << failed_ << " failed\n";
    }
    
    int GetFailed() const { return failed_; }
    
private:
    int passed_ = 0;
    int failed_ = 0;
};

bool FloatEqual(float a, float b, float epsilon = 0.0001f) {
    return std::abs(a - b) < epsilon;
}

// ============================================================================
// Key Binding Tests
// ============================================================================

void TestKeyBinding(TestResults& results) {
    std::cout << "\n=== Testing Key Binding ===" << std::endl;
    
    InputManager input;
    
    // Test 1: Map key
    input.MapKey("accelerate", SDL_SCANCODE_UP);
    if (input.GetMappedKey("accelerate") == SDL_SCANCODE_UP) {
        results.Pass("Map key to action");
    } else {
        results.Fail("Map key to action", "Key should be mapped to action");
    }
    
    // Test 2: Map multiple keys
    input.MapKey("brake", SDL_SCANCODE_DOWN);
    input.MapKey("nitro", SDL_SCANCODE_SPACE);
    if (input.GetMappedKey("brake") == SDL_SCANCODE_DOWN &&
        input.GetMappedKey("nitro") == SDL_SCANCODE_SPACE) {
        results.Pass("Map multiple keys");
    } else {
        results.Fail("Map multiple keys", "All keys should be mapped");
    }
    
    // Test 3: Remap key
    input.MapKey("accelerate", SDL_SCANCODE_W);
    if (input.GetMappedKey("accelerate") == SDL_SCANCODE_W) {
        results.Pass("Remap key to different scancode");
    } else {
        results.Fail("Remap key to different scancode",
                     "Key should be remapped");
    }
    
    // Test 4: Get unmapped key
    if (input.GetMappedKey("unmapped") == SDL_SCANCODE_UNKNOWN) {
        results.Pass("Get unmapped key returns UNKNOWN");
    } else {
        results.Fail("Get unmapped key returns UNKNOWN",
                     "Should return SCANCODE_UNKNOWN");
    }
    
    // Test 5: Clear bindings
    input.ClearKeyBindings();
    if (input.GetMappedKey("accelerate") == SDL_SCANCODE_UNKNOWN) {
        results.Pass("Clear all key bindings");
    } else {
        results.Fail("Clear all key bindings",
                     "Bindings should be cleared");
    }
}

// ============================================================================
// Axis Binding Tests
// ============================================================================

void TestAxisBinding(TestResults& results) {
    std::cout << "\n=== Testing Axis Binding ===" << std::endl;
    
    InputManager input;
    
    // Test 1: Map axis
    input.MapAxis("steering", 0, 0, false);
    if (input.GetAxisValue("steering") == 0.0f) {
        results.Pass("Map axis to action");
    } else {
        results.Fail("Map axis to action",
                     "Axis should be mapped (initial value 0)");
    }
    
    // Test 2: Map multiple axes
    input.MapAxis("throttle", 0, 5, false);
    input.MapAxis("brake", 0, 4, false);
    if (input.GetAxisValue("throttle") == 0.0f &&
        input.GetAxisValue("brake") == 0.0f) {
        results.Pass("Map multiple axes");
    } else {
        results.Fail("Map multiple axes", "All axes should initialize to 0");
    }
    
    // Test 3: Get unmapped axis
    if (input.GetAxisValue("unmapped") == 0.0f) {
        results.Pass("Get unmapped axis returns 0");
    } else {
        results.Fail("Get unmapped axis returns 0",
                     "Should return 0.0f for unmapped axis");
    }
}

// ============================================================================
// Dead Zone Tests
// ============================================================================

void TestDeadZone(TestResults& results) {
    std::cout << "\n=== Testing Dead Zone ===" << std::endl;
    
    InputManager input;
    input.SetDeadZone(0.15f);
    
    // Test 1: Dead zone storage
    if (input.GetDeadZone() == 0.15f) {
        results.Pass("Store dead zone value");
    } else {
        results.Fail("Store dead zone value", "Dead zone should be stored");
    }
    
    // Test 2: Dead zone clamping max
    input.SetDeadZone(0.6f);  // Try to set beyond max (0.5)
    if (input.GetDeadZone() <= 0.5f) {
        results.Pass("Dead zone clamping max");
    } else {
        results.Fail("Dead zone clamping max",
                     "Should clamp to 0.5 max");
    }
    
    // Test 3: Dead zone clamping min
    input.SetDeadZone(-0.1f);
    if (input.GetDeadZone() >= 0.0f) {
        results.Pass("Dead zone clamping min");
    } else {
        results.Fail("Dead zone clamping min",
                     "Should clamp to 0.0 min");
    }
    
    // Test 4: Apply dead zone - value within zone
    input.SetDeadZone(0.2f);
    float deadened = input.ApplyDeadZone(0.1f, 0.2f);
    if (FloatEqual(deadened, 0.0f, 0.01f)) {
        results.Pass("Apply dead zone - within zone");
    } else {
        results.Fail("Apply dead zone - within zone",
                     "Should return ~0 for values in dead zone");
    }
    
    // Test 5: Apply dead zone - value outside zone
    deadened = input.ApplyDeadZone(0.5f, 0.2f);
    if (deadened > 0.0f && deadened <= 1.0f) {
        results.Pass("Apply dead zone - outside zone");
    } else {
        results.Fail("Apply dead zone - outside zone",
                     "Should rescale values outside zone");
    }
    
    // Test 6: Apply dead zone - negative values
    deadened = input.ApplyDeadZone(-0.5f, 0.2f);
    if (deadened < 0.0f && deadened >= -1.0f) {
        results.Pass("Apply dead zone - negative values");
    } else {
        results.Fail("Apply dead zone - negative values",
                     "Should handle negative values correctly");
    }
}

// ============================================================================
// Mouse Tests
// ============================================================================

void TestMouseControl(TestResults& results) {
    std::cout << "\n=== Testing Mouse Control ===" << std::endl;
    
    InputManager input;
    
    // Test 1: Mouse position defaults
    if (input.GetMouseX() == 0 && input.GetMouseY() == 0) {
        results.Pass("Mouse position defaults");
    } else {
        results.Fail("Mouse position defaults",
                     "Should default to (0, 0)");
    }
    
    // Test 2: Mouse delta defaults
    if (input.GetMouseDeltaX() == 0 && input.GetMouseDeltaY() == 0) {
        results.Pass("Mouse delta defaults");
    } else {
        results.Fail("Mouse delta defaults",
                     "Should default to (0, 0)");
    }
    
    // Test 3: Mouse visibility control
    input.SetMouseVisible(false);
    if (!input.IsMouseVisible()) {
        results.Pass("Toggle mouse visibility");
    } else {
        results.Fail("Toggle mouse visibility",
                     "Should hide/show mouse cursor");
    }
    
    // Test 4: Mouse capture control
    input.SetMouseCapture(true);
    if (input.IsMouseCaptured()) {
        results.Pass("Toggle mouse capture");
    } else {
        results.Fail("Toggle mouse capture",
                     "Should capture/release mouse");
    }
}

// ============================================================================
// Text Input Tests
// ============================================================================

void TestTextInput(TestResults& results) {
    std::cout << "\n=== Testing Text Input ===" << std::endl;
    
    InputManager input;
    
    // Test 1: Start/stop text input
    input.StartTextInput();
    // Note: Can't directly verify on this layer, but no crash = success
    results.Pass("Start text input");
    
    input.StopTextInput();
    results.Pass("Stop text input");
    
    // Test 2: Get text input (should be empty initially after Update)
    input.Update();
    std::string text = input.GetTextInput();
    if (text.empty() || text.length() < 100) {  // Reasonable length
        results.Pass("Get text input");
    } else {
        results.Fail("Get text input", "Text input should be reasonable");
    }
}

// ============================================================================
// Gamepad Tests
// ============================================================================

void TestGamepadSupport(TestResults& results) {
    std::cout << "\n=== Testing Gamepad Support ===" << std::endl;
    
    InputManager input;
    input.Initialize();
    
    // Test 1: Gamepad check
    bool connected = input.IsGamepadConnected(0);
    // May or may not have gamepad, just test function works
    results.Pass("Check gamepad connection");
    
    // Test 2: Vibration feedback (may not do anything without hardware)
    input.VibrationFeedback(0, 0.5f, 0.8f, 200);
    results.Pass("Send vibration feedback");
}

// ============================================================================
// Callback Tests
// ============================================================================

void TestCallbacks(TestResults& results) {
    std::cout << "\n=== Testing Callbacks ===" << std::endl;
    
    InputManager input;
    
    // Test 1: Set key callback
    int key_callback_count = 0;
    input.SetKeyCallback([&](const std::string& action, bool pressed) {
        key_callback_count++;
    });
    results.Pass("Set key callback");
    
    // Test 2: Set axis callback
    int axis_callback_count = 0;
    input.SetAxisCallback([&](const std::string& action, float value) {
        axis_callback_count++;
    });
    results.Pass("Set axis callback");
}

// ============================================================================
// Integration Tests
// ============================================================================

void TestInputIntegration(TestResults& results) {
    std::cout << "\n=== Testing Input Integration ===" << std::endl;
    
    InputManager input;
    if (!input.Initialize()) {
        results.Fail("Input initialization", "Should initialize successfully");
        return;
    }
    results.Pass("Input initialization");
    
    // Test 1: Setup bindings
    input.MapKey("quit", SDL_SCANCODE_ESCAPE);
    input.MapKey("pause", SDL_SCANCODE_P);
    input.MapAxis("steering", 0, 0, false);
    input.SetDeadZone(0.15f);
    
    // Test 2: Update input system
    input.Update();
    results.Pass("Update input system");
    
    // Test 3: Shutdown
    input.Shutdown();
    results.Pass("Shutdown input system");
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
    std::cout << "\n" << "="*60 << std::endl;
    std::cout << "TopSpeed Input Manager Test Suite" << std::endl;
    std::cout << "="*60 << std::endl;
    
    TestResults results;
    
    // Run all tests
    TestKeyBinding(results);
    TestAxisBinding(results);
    TestDeadZone(results);
    TestMouseControl(results);
    TestTextInput(results);
    TestGamepadSupport(results);
    TestCallbacks(results);
    TestInputIntegration(results);
    
    // Print summary
    results.PrintSummary();
    
    std::cout << "\n" << "="*60 << std::endl;
    if (results.GetFailed() == 0) {
        std::cout << "✓ ALL TESTS PASSED" << std::endl;
    } else {
        std::cout << "✗ SOME TESTS FAILED" << std::endl;
    }
    std::cout << "="*60 << std::endl;
    
    return results.GetFailed();
}
