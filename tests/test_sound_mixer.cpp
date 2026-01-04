#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
#include "../src/audio/SoundMixer.h"
#include "../src/audio/Effects.h"

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
// SoundMixer Tests
// ============================================================================

void TestSoundMixerBasics(TestResults& results) {
    std::cout << "\n=== Testing SoundMixer Basics ===" << std::endl;
    
    SoundMixer mixer("test_mixer");
    
    // Test 1: Created with correct name
    if (mixer.GetName() == "test_mixer") {
        results.Pass("Mixer name storage");
    } else {
        results.Fail("Mixer name storage", "Name should be stored correctly");
    }
    
    // Test 2: Default volume is 1.0
    if (FloatEqual(mixer.GetVolume(), 1.0f)) {
        results.Pass("Mixer default volume");
    } else {
        results.Fail("Mixer default volume", "Should default to 1.0");
    }
    
    // Test 3: Default enabled state
    if (mixer.IsEnabled()) {
        results.Pass("Mixer default enabled");
    } else {
        results.Fail("Mixer default enabled", "Should be enabled by default");
    }
    
    // Test 4: Default not muted
    if (!mixer.IsMuted()) {
        results.Pass("Mixer default not muted");
    } else {
        results.Fail("Mixer default not muted", "Should not be muted by default");
    }
}

void TestSoundMixerSounds(TestResults& results) {
    std::cout << "\n=== Testing SoundMixer Sound Management ===" << std::endl;
    
    SoundMixer mixer("music");
    
    // Test 1: Add sound
    mixer.AddSound(1001);
    if (mixer.HasSound(1001)) {
        results.Pass("Add sound to mixer");
    } else {
        results.Fail("Add sound to mixer", "Sound should be added");
    }
    
    // Test 2: Add multiple sounds
    mixer.AddSound(1002);
    mixer.AddSound(1003);
    if (mixer.HasSound(1002) && mixer.HasSound(1003)) {
        results.Pass("Add multiple sounds");
    } else {
        results.Fail("Add multiple sounds", "All sounds should be added");
    }
    
    // Test 3: Duplicate add is ignored
    int initial_count = mixer.GetSoundCount();
    mixer.AddSound(1001);  // Already added
    if (mixer.GetSoundCount() == initial_count) {
        results.Pass("Duplicate sound ignored");
    } else {
        results.Fail("Duplicate sound ignored", "Duplicate should not increase count");
    }
    
    // Test 4: Remove sound
    mixer.RemoveSound(1002);
    if (!mixer.HasSound(1002)) {
        results.Pass("Remove sound from mixer");
    } else {
        results.Fail("Remove sound from mixer", "Sound should be removed");
    }
    
    // Test 5: Clear all sounds
    mixer.ClearSounds();
    if (mixer.GetSoundCount() == 0) {
        results.Pass("Clear all sounds");
    } else {
        results.Fail("Clear all sounds", "All sounds should be cleared");
    }
}

void TestSoundMixerVolume(TestResults& results) {
    std::cout << "\n=== Testing SoundMixer Volume Control ===" << std::endl;
    
    SoundMixer mixer("sfx");
    
    // Test 1: Set volume
    mixer.SetVolume(0.8f);
    if (FloatEqual(mixer.GetVolume(), 0.8f)) {
        results.Pass("Set mixer volume");
    } else {
        results.Fail("Set mixer volume", "Volume should be set correctly");
    }
    
    // Test 2: Volume clamping (max 1.0)
    mixer.SetVolume(1.5f);
    if (mixer.GetVolume() <= 1.0f) {
        results.Pass("Volume clamping max");
    } else {
        results.Fail("Volume clamping max", "Should clamp to 1.0");
    }
    
    // Test 3: Volume clamping (min 0.0)
    mixer.SetVolume(-0.5f);
    if (mixer.GetVolume() >= 0.0f) {
        results.Pass("Volume clamping min");
    } else {
        results.Fail("Volume clamping min", "Should clamp to 0.0");
    }
    
    // Test 4: Fade volume
    mixer.SetVolume(1.0f);  // Reset
    mixer.FadeVolume(0.5f, 1000);  // Fade to 50% in 1 second
    
    // Update clock should show fade in progress
    mixer.Update(0.1f);  // 100ms
    if (mixer.GetVolume() < 1.0f && mixer.GetVolume() > 0.5f) {
        results.Pass("Fade volume in progress");
    } else {
        results.Fail("Fade volume in progress", "Should be between start and target");
    }
    
    // Test 5: Fade completion
    mixer.Update(1.0f);  // Another 1 second
    if (FloatEqual(mixer.GetVolume(), 0.5f, 0.05f)) {
        results.Pass("Fade volume completion");
    } else {
        results.Fail("Fade volume completion", "Should reach target volume");
    }
}

void TestSoundMixerEffects(TestResults& results) {
    std::cout << "\n=== Testing SoundMixer Effect Chain ===" << std::endl;
    
    SoundMixer mixer("music");
    
    // Test 1: Add effect
    mixer.AddEffect(std::make_unique<ReverbEffect>(50.0f, 0.3f));
    if (mixer.GetEffectCount() == 1) {
        results.Pass("Add effect to mixer");
    } else {
        results.Fail("Add effect to mixer", "Effect should be added");
    }
    
    // Test 2: Add multiple effects
    mixer.AddEffect(std::make_unique<EqualizerEffect>());
    mixer.AddEffect(std::make_unique<CompressorEffect>(0.5f, 4.0f, 0.1f, 0.2f));
    if (mixer.GetEffectCount() == 3) {
        results.Pass("Add multiple effects");
    } else {
        results.Fail("Add multiple effects", "All effects should be added");
    }
    
    // Test 3: Get effect
    AudioEffect* effect = mixer.GetEffect(0);
    if (effect != nullptr) {
        results.Pass("Get effect from mixer");
    } else {
        results.Fail("Get effect from mixer", "Should return effect pointer");
    }
    
    // Test 4: Get non-existent effect
    effect = mixer.GetEffect(100);  // Out of bounds
    if (effect == nullptr) {
        results.Pass("Get non-existent effect returns null");
    } else {
        results.Fail("Get non-existent effect returns null",
                     "Should return nullptr for invalid index");
    }
    
    // Test 5: Remove effect
    mixer.RemoveEffect(1);
    if (mixer.GetEffectCount() == 2) {
        results.Pass("Remove effect from mixer");
    } else {
        results.Fail("Remove effect from mixer", "Effect should be removed");
    }
    
    // Test 6: Clear all effects
    mixer.ClearEffects();
    if (mixer.GetEffectCount() == 0) {
        results.Pass("Clear all effects");
    } else {
        results.Fail("Clear all effects", "All effects should be cleared");
    }
}

void TestSoundMixerProcessing(TestResults& results) {
    std::cout << "\n=== Testing SoundMixer Sample Processing ===" << std::endl;
    
    SoundMixer mixer("test");
    
    // Test 1: Process without effects
    float output = mixer.ProcessSample(0.5f);
    if (FloatEqual(output, 0.5f)) {
        results.Pass("Process sample basic");
    } else {
        results.Fail("Process sample basic", "Should output correct volume");
    }
    
    // Test 2: Process with effect
    mixer.AddEffect(std::make_unique<ReverbEffect>(50.0f, 0.3f));
    output = mixer.ProcessSample(0.5f);
    if (output >= 0.0f && output <= 1.0f) {
        results.Pass("Process sample with effect");
    } else {
        results.Fail("Process sample with effect", "Output should be in valid range");
    }
    
    // Test 3: Disabled mixer returns zero
    mixer.SetEnabled(false);
    output = mixer.ProcessSample(0.5f);
    if (FloatEqual(output, 0.0f, 0.001f)) {
        results.Pass("Disabled mixer returns zero");
    } else {
        results.Fail("Disabled mixer returns zero", "Should return 0 when disabled");
    }
    
    // Test 4: Muted mixer returns zero
    mixer.SetEnabled(true);
    mixer.SetMuted(true);
    output = mixer.ProcessSample(0.5f);
    if (FloatEqual(output, 0.0f, 0.001f)) {
        results.Pass("Muted mixer returns zero");
    } else {
        results.Fail("Muted mixer returns zero", "Should return 0 when muted");
    }
    
    // Test 5: Volume multiplication
    mixer.SetMuted(false);
    mixer.SetVolume(0.5f);
    output = mixer.ProcessSample(1.0f);
    if (FloatEqual(output, 0.5f, 0.05f)) {
        results.Pass("Volume multiplication");
    } else {
        results.Fail("Volume multiplication", "Output should be scaled by volume");
    }
}

// ============================================================================
// MixerManager Tests
// ============================================================================

void TestMixerManager(TestResults& results) {
    std::cout << "\n=== Testing MixerManager ===" << std::endl;
    
    MixerManager manager;
    
    // Test 1: Create mixer
    SoundMixer* mixer = manager.CreateMixer("test");
    if (mixer != nullptr && mixer->GetName() == "test") {
        results.Pass("Create mixer in manager");
    } else {
        results.Fail("Create mixer in manager", "Mixer should be created");
    }
    
    // Test 2: Get existing mixer
    SoundMixer* retrieved = manager.GetMixer("test");
    if (retrieved == mixer) {
        results.Pass("Get existing mixer");
    } else {
        results.Fail("Get existing mixer", "Should return same mixer instance");
    }
    
    // Test 3: Get non-existent mixer
    SoundMixer* missing = manager.GetMixer("missing");
    if (missing == nullptr) {
        results.Pass("Get non-existent mixer returns null");
    } else {
        results.Fail("Get non-existent mixer returns null", "Should return nullptr");
    }
    
    // Test 4: Create duplicate mixer returns existing
    manager.CreateMixer("dup");
    SoundMixer* dup2 = manager.CreateMixer("dup");
    if (dup2 == manager.GetMixer("dup")) {
        results.Pass("Duplicate mixer returns existing");
    } else {
        results.Fail("Duplicate mixer returns existing",
                     "Should return same instance");
    }
    
    // Test 5: Master volume control
    manager.SetMasterVolume(0.8f);
    if (FloatEqual(manager.GetMasterVolume(), 0.8f)) {
        results.Pass("Master volume control");
    } else {
        results.Fail("Master volume control", "Master volume should be set");
    }
    
    // Test 6: Remove mixer
    manager.RemoveMixer("test");
    if (manager.GetMixer("test") == nullptr) {
        results.Pass("Remove mixer from manager");
    } else {
        results.Fail("Remove mixer from manager", "Mixer should be removed");
    }
    
    // Test 7: Clear all mixers
    manager.ClearMixers();
    if (manager.GetMixer("dup") == nullptr) {
        results.Pass("Clear all mixers");
    } else {
        results.Fail("Clear all mixers", "All mixers should be cleared");
    }
}

void TestMixerManagerProcessing(TestResults& results) {
    std::cout << "\n=== Testing MixerManager Sample Processing ===" << std::endl;
    
    MixerManager manager;
    
    // Create multiple mixers
    SoundMixer* music = manager.CreateMixer("music");
    SoundMixer* sfx = manager.CreateMixer("sfx");
    
    music->SetVolume(0.8f);
    sfx->SetVolume(0.9f);
    manager.SetMasterVolume(0.9f);
    
    // Test 1: Process through music mixer
    float output = manager.ProcessSample("music", 1.0f);
    // Expected: 1.0 * 0.8 (music) * 0.9 (master) = 0.72
    if (FloatEqual(output, 0.72f, 0.05f)) {
        results.Pass("Process through music mixer");
    } else {
        results.Fail("Process through music mixer",
                     "Should apply both mixer and master volume");
    }
    
    // Test 2: Process through sfx mixer
    output = manager.ProcessSample("sfx", 1.0f);
    // Expected: 1.0 * 0.9 (sfx) * 0.9 (master) = 0.81
    if (FloatEqual(output, 0.81f, 0.05f)) {
        results.Pass("Process through sfx mixer");
    } else {
        results.Fail("Process through sfx mixer",
                     "Should apply both mixer and master volume");
    }
    
    // Test 3: Process through non-existent mixer uses master volume only
    output = manager.ProcessSample("missing", 1.0f);
    if (FloatEqual(output, 0.9f, 0.05f)) {
        results.Pass("Process unknown mixer uses master volume");
    } else {
        results.Fail("Process unknown mixer uses master volume",
                     "Should apply master volume only");
    }
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
    std::cout << "\n" << "="*60 << std::endl;
    std::cout << "TopSpeed Sound Mixer Test Suite" << std::endl;
    std::cout << "="*60 << std::endl;
    
    TestResults results;
    
    // Run all tests
    TestSoundMixerBasics(results);
    TestSoundMixerSounds(results);
    TestSoundMixerVolume(results);
    TestSoundMixerEffects(results);
    TestSoundMixerProcessing(results);
    TestMixerManager(results);
    TestMixerManagerProcessing(results);
    
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
