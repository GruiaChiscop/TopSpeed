#pragma once

#include <cmath>
#include <algorithm>

namespace TopSpeed {

/**
 * Base class for audio effects
 * Processed in a mixer effect chain
 */
class AudioEffect {
public:
    virtual ~AudioEffect() = default;
    
    /**
     * Process audio sample through this effect
     * @param sample Input sample (-1.0 to 1.0)
     * @return Processed sample
     */
    virtual float Process(float sample) = 0;
    
    /**
     * Enable/disable this effect
     */
    virtual void SetEnabled(bool enabled) { enabled_ = enabled; }
    virtual bool IsEnabled() const { return enabled_; }
    
    /**
     * Bypass effect (return input unchanged)
     */
    virtual void Reset() {}

protected:
    bool enabled_ = true;
};

/**
 * Simple reverb effect using delay line
 * Creates spacious echo/room effect
 */
class ReverbEffect : public AudioEffect {
public:
    /**
     * Initialize reverb with buffer size
     * @param delay_ms Delay time in milliseconds (0-500ms)
     */
    ReverbEffect(float delay_ms = 50.0f, float decay = 0.5f);
    ~ReverbEffect();
    
    float Process(float sample) override;
    void Reset() override;
    
    /**
     * Set reverb parameters
     * @param delay_ms Delay in milliseconds
     * @param decay Decay factor (0.0-0.9)
     */
    void SetParameters(float delay_ms, float decay);
    
    /**
     * Set wet/dry mix
     * @param wet Amount of reverb (0.0 = dry, 1.0 = wet)
     */
    void SetWet(float wet) { wet_ = std::clamp(wet, 0.0f, 1.0f); }

private:
    std::vector<float> delay_buffer_;
    size_t write_pos_ = 0;
    size_t delay_samples_ = 0;
    float decay_ = 0.5f;
    float wet_ = 0.5f;
    float last_output_ = 0.0f;
};

/**
 * 3-band equalizer (Low, Mid, High)
 */
class EqualizerEffect : public AudioEffect {
public:
    EqualizerEffect();
    
    float Process(float sample) override;
    void Reset() override;
    
    /**
     * Set band gain in dB
     * @param band 0=Low, 1=Mid, 2=High
     * @param db Gain in dB (-12 to +12)
     */
    void SetBandGain(int band, float db);
    
    /**
     * Get band gain
     */
    float GetBandGain(int band) const;

private:
    // Simple one-pole low-pass for each band
    float band_gains_[3] = {0.0f, 0.0f, 0.0f};
    float low_state_ = 0.0f;
    float mid_state_ = 0.0f;
    float high_state_ = 0.0f;
};

/**
 * Dynamic range compressor
 * Reduces volume of loud parts, amplifies quiet parts
 */
class CompressorEffect : public AudioEffect {
public:
    CompressorEffect();
    
    float Process(float sample) override;
    void Reset() override;
    
    /**
     * Set compressor parameters
     * @param threshold Threshold above which compression starts (0.0-1.0)
     * @param ratio Compression ratio (1:ratio)
     * @param attack_ms Attack time in milliseconds
     * @param release_ms Release time in milliseconds
     */
    void SetParameters(float threshold, float ratio, float attack_ms, float release_ms);

private:
    float threshold_ = 0.7f;
    float ratio_ = 4.0f;
    float attack_coeff_ = 0.005f;
    float release_coeff_ = 0.001f;
    float envelope_ = 0.0f;
};

/**
 * Distortion effect for special audio (crash, damage)
 */
class DistortionEffect : public AudioEffect {
public:
    DistortionEffect(float drive = 1.0f);
    
    float Process(float sample) override;
    
    /**
     * Set distortion drive amount
     * @param drive Drive amount (1.0 = none, 10.0 = heavy)
     */
    void SetDrive(float drive) { drive_ = std::max(1.0f, drive); }

private:
    float drive_ = 1.0f;
    
    // Soft clipping function
    float SoftClip(float x);
};

/**
 * Flanging effect (slight pitch modulation)
 * Creates "whooshing" sound
 */
class FlangeEffect : public AudioEffect {
public:
    FlangeEffect(float rate_hz = 0.5f);
    
    float Process(float sample) override;
    void Reset() override;
    
    /**
     * Set flange rate
     * @param rate_hz LFO rate in Hz (0.1-10 Hz)
     */
    void SetRate(float rate_hz) { rate_hz_ = std::clamp(rate_hz, 0.1f, 10.0f); }
    
    /**
     * Set flange depth
     * @param depth Modulation depth (0.0-1.0)
     */
    void SetDepth(float depth) { depth_ = std::clamp(depth, 0.0f, 1.0f); }

private:
    std::vector<float> delay_buffer_;
    size_t write_pos_ = 0;
    float phase_ = 0.0f;
    float rate_hz_ = 0.5f;
    float depth_ = 0.5f;
    size_t max_delay_samples_ = 1024;
};

} // namespace TopSpeed
