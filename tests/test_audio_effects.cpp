#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
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
// Reverb Effect Tests
// ============================================================================

void TestReverbEffect(TestResults& results) {
    std::cout << "\n=== Testing ReverbEffect ===" << std::endl;
    
    ReverbEffect reverb(50.0f, 0.5f);
    
    // Test 1: Effect is enabled by default
    if (reverb.IsEnabled()) {
        results.Pass("Reverb enabled by default");
    } else {
        results.Fail("Reverb enabled by default", "Effect should be enabled");
    }
    
    // Test 2: Process samples
    float input = 0.5f;
    float output = reverb.Process(input);
    if (output >= 0.0f && output <= 1.0f) {
        results.Pass("Reverb output in valid range");
    } else {
        results.Fail("Reverb output in valid range", 
                     "Output should be between 0 and 1");
    }
    
    // Test 3: Silence produces silence
    output = reverb.Process(0.0f);
    if (FloatEqual(output, 0.0f, 0.001f)) {
        results.Pass("Reverb silence handling");
    } else {
        results.Fail("Reverb silence handling", "Silent input should produce ~0 output");
    }
    
    // Test 4: Reset works
    reverb.Reset();
    output = reverb.Process(0.5f);
    if (output >= 0.0f && output <= 1.0f) {
        results.Pass("Reverb reset works");
    } else {
        results.Fail("Reverb reset works", "Reset should clear internal state");
    }
    
    // Test 5: Disable effect
    reverb.SetEnabled(false);
    output = reverb.Process(0.5f);
    if (FloatEqual(output, 0.5f, 0.001f)) {
        results.Pass("Reverb disable passthrough");
    } else {
        results.Fail("Reverb disable passthrough", 
                     "Disabled effect should pass through unchanged");
    }
}

// ============================================================================
// Equalizer Effect Tests
// ============================================================================

void TestEqualizerEffect(TestResults& results) {
    std::cout << "\n=== Testing EqualizerEffect ===" << std::endl;
    
    EqualizerEffect eq;
    
    // Test 1: Default gains are neutral
    if (FloatEqual(eq.GetBandGain(0), 0.0f) &&
        FloatEqual(eq.GetBandGain(1), 0.0f) &&
        FloatEqual(eq.GetBandGain(2), 0.0f)) {
        results.Pass("EQ default neutral gains");
    } else {
        results.Fail("EQ default neutral gains", 
                     "Default gains should be 0dB");
    }
    
    // Test 2: Set band gains
    eq.SetBandGain(0, 6.0f);  // Boost bass
    if (FloatEqual(eq.GetBandGain(0), 6.0f)) {
        results.Pass("EQ set band gain");
    } else {
        results.Fail("EQ set band gain", "Should store band gain value");
    }
    
    // Test 3: Gain clamping (max 12dB)
    eq.SetBandGain(1, 20.0f);  // Try to set beyond max
    if (eq.GetBandGain(1) <= 12.0f) {
        results.Pass("EQ gain clamping max");
    } else {
        results.Fail("EQ gain clamping max", "Should clamp to 12dB max");
    }
    
    // Test 4: Gain clamping (min -12dB)
    eq.SetBandGain(2, -20.0f);  // Try to set beyond min
    if (eq.GetBandGain(2) >= -12.0f) {
        results.Pass("EQ gain clamping min");
    } else {
        results.Fail("EQ gain clamping min", "Should clamp to -12dB min");
    }
    
    // Test 5: Process with neutral gains
    eq.SetBandGain(0, 0.0f);
    eq.SetBandGain(1, 0.0f);
    eq.SetBandGain(2, 0.0f);
    float output = eq.Process(0.5f);
    if (FloatEqual(output, 0.5f, 0.01f)) {
        results.Pass("EQ neutral processing");
    } else {
        results.Fail("EQ neutral processing", 
                     "Neutral EQ should pass through nearly unchanged");
    }
}

// ============================================================================
// Compressor Effect Tests
// ============================================================================

void TestCompressorEffect(TestResults& results) {
    std::cout << "\n=== Testing CompressorEffect ===" << std::endl;
    
    CompressorEffect compressor(0.5f, 4.0f, 0.1f, 0.2f);
    
    // Test 1: Effect is enabled by default
    if (compressor.IsEnabled()) {
        results.Pass("Compressor enabled by default");
    } else {
        results.Fail("Compressor enabled by default", "Effect should be enabled");
    }
    
    // Test 2: Below threshold passes through
    float quiet = 0.2f;  // Below 0.5 threshold
    float output = compressor.Process(quiet);
    if (FloatEqual(output, quiet, 0.05f)) {
        results.Pass("Compressor below threshold");
    } else {
        results.Fail("Compressor below threshold", 
                     "Signal below threshold should pass through");
    }
    
    // Test 3: Above threshold gets compressed
    compressor.Reset();
    float loud = 0.8f;  // Above 0.5 threshold
    output = compressor.Process(loud);
    if (output < loud) {
        results.Pass("Compressor above threshold reduces level");
    } else {
        results.Fail("Compressor above threshold reduces level",
                     "Loud signal should be reduced");
    }
    
    // Test 4: Compression is gentle (ratio 4:1)
    compressor.Reset();
    output = compressor.Process(0.9f);
    if (output > 0.6f && output < 0.9f) {
        results.Pass("Compressor ratio effect");
    } else {
        results.Fail("Compressor ratio effect", 
                     "Compression should be moderate");
    }
    
    // Test 5: Output is in valid range
    if (output >= 0.0f && output <= 1.0f) {
        results.Pass("Compressor output clamping");
    } else {
        results.Fail("Compressor output clamping",
                     "Output should stay in 0-1 range");
    }
}

// ============================================================================
// Distortion Effect Tests
// ============================================================================

void TestDistortionEffect(TestResults& results) {
    std::cout << "\n=== Testing DistortionEffect ===" << std::endl;
    
    DistortionEffect distortion(0.5f);
    
    // Test 1: Zero drive is passthrough
    DistortionEffect clean(0.0f);
    float output = clean.Process(0.5f);
    if (FloatEqual(output, 0.5f, 0.001f)) {
        results.Pass("Distortion zero drive passthrough");
    } else {
        results.Fail("Distortion zero drive passthrough",
                     "Zero drive should not affect signal");
    }
    
    // Test 2: Increased drive affects signal
    output = distortion.Process(0.5f);
    if (!FloatEqual(output, 0.5f, 0.01f)) {
        results.Pass("Distortion drive affects signal");
    } else {
        results.Fail("Distortion drive affects signal",
                     "Non-zero drive should alter the signal");
    }
    
    // Test 3: Max drive (1.0) creates strong effect
    DistortionEffect heavy(1.0f);
    float clean_signal = 0.9f;
    float dirty_signal = heavy.Process(clean_signal);
    if (dirty_signal != clean_signal) {
        results.Pass("Distortion max drive effect");
    } else {
        results.Fail("Distortion max drive effect",
                     "Max drive should significantly alter signal");
    }
    
    // Test 4: Output is in valid range
    if (dirty_signal >= -1.0f && dirty_signal <= 1.0f) {
        results.Pass("Distortion output range");
    } else {
        results.Fail("Distortion output range",
                     "Output should be in -1 to 1 range");
    }
    
    // Test 5: Reset clears state
    distortion.Reset();
    output = distortion.Process(0.5f);
    if (output >= -1.0f && output <= 1.0f) {
        results.Pass("Distortion reset works");
    } else {
        results.Fail("Distortion reset works",
                     "Reset should initialize effect properly");
    }
}

// ============================================================================
// Flange Effect Tests
// ============================================================================

void TestFlangeEffect(TestResults& results) {
    std::cout << "\n=== Testing FlangeEffect ===" << std::endl;
    
    FlangeEffect flange(2.0f, 0.5f);
    
    // Test 1: Effect is enabled by default
    if (flange.IsEnabled()) {
        results.Pass("Flange enabled by default");
    } else {
        results.Fail("Flange enabled by default", "Effect should be enabled");
    }
    
    // Test 2: Process produces output
    float output = flange.Process(0.5f);
    if (output >= -1.0f && output <= 1.0f) {
        results.Pass("Flange output valid range");
    } else {
        results.Fail("Flange output valid range",
                     "Output should be in valid range");
    }
    
    // Test 3: LFO modulation creates variation
    std::vector<float> outputs;
    for (int i = 0; i < 100; i++) {
        outputs.push_back(flange.Process(0.5f));
    }
    
    float min_val = outputs[0];
    float max_val = outputs[0];
    for (float v : outputs) {
        if (v < min_val) min_val = v;
        if (v > max_val) max_val = v;
    }
    
    if (max_val > min_val) {
        results.Pass("Flange LFO modulation");
    } else {
        results.Fail("Flange LFO modulation",
                     "Output should vary over time");
    }
    
    // Test 4: Reset works
    flange.Reset();
    output = flange.Process(0.5f);
    if (output >= -1.0f && output <= 1.0f) {
        results.Pass("Flange reset works");
    } else {
        results.Fail("Flange reset works",
                     "Reset should initialize properly");
    }
    
    // Test 5: Disabled passthrough
    flange.SetEnabled(false);
    output = flange.Process(0.5f);
    if (FloatEqual(output, 0.5f, 0.001f)) {
        results.Pass("Flange disabled passthrough");
    } else {
        results.Fail("Flange disabled passthrough",
                     "Disabled effect should pass through");
    }
}

// ============================================================================
// Effect Chain Tests
// ============================================================================

void TestEffectChain(TestResults& results) {
    std::cout << "\n=== Testing Effect Chain Processing ===" << std::endl;
    
    // Create a chain: Distortion -> EQ -> Compressor
    std::vector<std::unique_ptr<AudioEffect>> chain;
    chain.push_back(std::make_unique<DistortionEffect>(0.3f));
    chain.push_back(std::make_unique<EqualizerEffect>());
    chain.push_back(std::make_unique<CompressorEffect>(0.6f, 4.0f, 0.1f, 0.2f));
    
    // Test 1: Process through chain
    float input = 0.5f;
    float output = input;
    for (auto& effect : chain) {
        output = effect->Process(output);
    }
    
    if (output >= -1.0f && output <= 1.0f) {
        results.Pass("Effect chain output valid");
    } else {
        results.Fail("Effect chain output valid",
                     "Chain output should be in valid range");
    }
    
    // Test 2: Different input levels
    for (float level : {0.1f, 0.3f, 0.5f, 0.7f, 0.9f}) {
        output = level;
        for (auto& effect : chain) {
            output = effect->Process(output);
        }
        if (!(output >= -1.0f && output <= 1.0f)) {
            results.Fail("Effect chain multi-level test",
                        "Output invalid at level " + std::to_string(level));
            return;
        }
    }
    results.Pass("Effect chain multi-level test");
    
    // Test 3: Enable/disable effects in chain
    chain[0]->SetEnabled(false);  // Disable distortion
    output = 0.5f;
    for (auto& effect : chain) {
        output = effect->Process(output);
    }
    if (output >= -1.0f && output <= 1.0f) {
        results.Pass("Effect chain selective disable");
    } else {
        results.Fail("Effect chain selective disable",
                     "Should still work with effects disabled");
    }
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
    std::cout << "\n" << "="*60 << std::endl;
    std::cout << "TopSpeed Audio Effects Test Suite" << std::endl;
    std::cout << "="*60 << std::endl;
    
    TestResults results;
    
    // Run all tests
    TestReverbEffect(results);
    TestEqualizerEffect(results);
    TestCompressorEffect(results);
    TestDistortionEffect(results);
    TestFlangeEffect(results);
    TestEffectChain(results);
    
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
