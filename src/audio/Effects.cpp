#include "Effects.h"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace TopSpeed {

// ============================================================================
// ReverbEffect Implementation
// ============================================================================

ReverbEffect::ReverbEffect(float delay_ms, float decay) : decay_(decay), wet_(0.5f) {
    SetParameters(delay_ms, decay);
}

ReverbEffect::~ReverbEffect() = default;

void ReverbEffect::SetParameters(float delay_ms, float decay) {
    // Calculate sample count from milliseconds (assuming 44100 Hz sample rate)
    const float sample_rate = 44100.0f;
    delay_samples_ = static_cast<size_t>(delay_ms * sample_rate / 1000.0f);
    delay_samples_ = std::min(delay_samples_, size_t(22050));  // Max 500ms at 44.1kHz
    
    decay_ = std::clamp(decay, 0.0f, 0.9f);
    
    // Allocate/resize delay buffer
    if (delay_buffer_.size() != delay_samples_) {
        delay_buffer_.resize(delay_samples_, 0.0f);
        write_pos_ = 0;
    }
}

void ReverbEffect::Reset() {
    std::fill(delay_buffer_.begin(), delay_buffer_.end(), 0.0f);
    write_pos_ = 0;
    last_output_ = 0.0f;
}

float ReverbEffect::Process(float sample) {
    if (!enabled_ || delay_samples_ == 0) {
        return sample;
    }
    
    // Read from delay buffer
    float delayed = delay_buffer_[write_pos_];
    
    // Feedback: write input + delayed * decay into buffer
    float feedback = sample + delayed * decay_;
    delay_buffer_[write_pos_] = feedback;
    
    // Move write pointer
    write_pos_ = (write_pos_ + 1) % delay_samples_;
    
    // Mix dry and wet
    float output = sample * (1.0f - wet_) + delayed * wet_;
    last_output_ = output;
    
    return output;
}

// ============================================================================
// EqualizerEffect Implementation
// ============================================================================

EqualizerEffect::EqualizerEffect() {
    // Default: neutral (0dB)
    band_gains_[0] = 0.0f;  // Low
    band_gains_[1] = 0.0f;  // Mid
    band_gains_[2] = 0.0f;  // High
}

void EqualizerEffect::SetBandGain(int band, float db) {
    if (band >= 0 && band < 3) {
        // Convert dB to linear gain
        // gain = 10^(dB/20)
        band_gains_[band] = std::pow(10.0f, db / 20.0f);
    }
}

float EqualizerEffect::GetBandGain(int band) const {
    if (band >= 0 && band < 3) {
        // Convert back to dB
        return 20.0f * std::log10(band_gains_[band]);
    }
    return 0.0f;
}

float EqualizerEffect::Process(float sample) {
    if (!enabled_) {
        return sample;
    }
    
    // Simple one-pole low-pass filter per band
    const float alpha = 0.1f;  // Filter coefficient
    
    // Low band (bass boost/cut)
    low_state_ += alpha * (sample * band_gains_[0] - low_state_);
    
    // Mid band
    mid_state_ += alpha * (sample * band_gains_[1] - mid_state_);
    
    // High band (treble boost/cut)
    high_state_ += alpha * (sample * band_gains_[2] - high_state_);
    
    // Mix the three bands
    float output = low_state_ + mid_state_ + high_state_;
    
    // Normalize to prevent clipping
    return std::clamp(output / 3.0f, -1.0f, 1.0f);
}

void EqualizerEffect::Reset() {
    low_state_ = 0.0f;
    mid_state_ = 0.0f;
    high_state_ = 0.0f;
}

// ============================================================================
// CompressorEffect Implementation
// ============================================================================

CompressorEffect::CompressorEffect() {
    SetParameters(0.7f, 4.0f, 10.0f, 100.0f);
}

void CompressorEffect::SetParameters(float threshold, float ratio, float attack_ms, float release_ms) {
    threshold_ = std::clamp(threshold, 0.0f, 1.0f);
    ratio_ = std::max(1.0f, ratio);
    
    // Convert attack/release time to exponential coefficients
    // At 44100 Hz sample rate
    attack_coeff_ = std::exp(-1.0f / (attack_ms * 44.1f));     // Fast attack
    release_coeff_ = std::exp(-1.0f / (release_ms * 44.1f));   // Slower release
}

float CompressorEffect::Process(float sample) {
    if (!enabled_) {
        return sample;
    }
    
    float input_level = std::abs(sample);
    
    // Envelope follower with attack/release
    if (input_level > envelope_) {
        // Attack phase (fast)
        envelope_ = attack_coeff_ * envelope_ + (1.0f - attack_coeff_) * input_level;
    } else {
        // Release phase (slow)
        envelope_ = release_coeff_ * envelope_ + (1.0f - release_coeff_) * input_level;
    }
    
    // Calculate gain reduction
    float gain = 1.0f;
    if (envelope_ > threshold_) {
        // Above threshold: apply compression
        float overshoot = envelope_ - threshold_;
        float reduction = overshoot * (1.0f - 1.0f / ratio_);
        gain = (envelope_ - reduction) / envelope_;
    }
    
    return sample * gain;
}

void CompressorEffect::Reset() {
    envelope_ = 0.0f;
}

// ============================================================================
// DistortionEffect Implementation
// ============================================================================

DistortionEffect::DistortionEffect(float drive) : drive_(std::max(1.0f, drive)) {}

float DistortionEffect::SoftClip(float x) {
    // Soft clipping using tanh approximation
    // tanh(x) ≈ x / (1 + |x|)  (fast approximation)
    if (x >= 0) {
        return x / (1.0f + x);
    } else {
        return x / (1.0f - x);
    }
}

float DistortionEffect::Process(float sample) {
    if (!enabled_) {
        return sample;
    }
    
    // Apply drive (amplification)
    float driven = sample * drive_;
    
    // Soft clip to prevent harsh clipping
    float clipped = SoftClip(driven);
    
    // Normalize output (reduce volume to compensate for drive)
    return clipped / (1.0f + std::log10(drive_));
}

// ============================================================================
// FlangeEffect Implementation
// ============================================================================

FlangeEffect::FlangeEffect(float rate_hz) : rate_hz_(std::clamp(rate_hz, 0.1f, 10.0f)) {
    // Initialize delay buffer
    delay_buffer_.resize(max_delay_samples_, 0.0f);
}

float FlangeEffect::Process(float sample) {
    if (!enabled_) {
        return sample;
    }
    
    // LFO (Low Frequency Oscillator) for modulation
    const float sample_rate = 44100.0f;
    float lfo_rate = rate_hz_ / sample_rate;
    phase_ += lfo_rate;
    if (phase_ > 1.0f) {
        phase_ -= 1.0f;
    }
    
    // Triangle wave modulation (0 to 1)
    float lfo = (phase_ < 0.5f) ? (phase_ * 2.0f) : (2.0f - phase_ * 2.0f);
    
    // Calculate delay time (varies between 2ms and 20ms)
    size_t delay_samples = static_cast<size_t>(2.0f + lfo * depth_ * 18.0f);
    delay_samples = std::min(delay_samples, max_delay_samples_ - 1);
    
    // Read from delay buffer
    size_t read_pos = (write_pos_ + max_delay_samples_ - delay_samples) % max_delay_samples_;
    float delayed = delay_buffer_[read_pos];
    
    // Write input to buffer
    delay_buffer_[write_pos_] = sample;
    write_pos_ = (write_pos_ + 1) % max_delay_samples_;
    
    // Mix dry and delayed signal
    float output = sample * 0.7f + delayed * 0.3f;
    
    return output;
}

void FlangeEffect::Reset() {
    std::fill(delay_buffer_.begin(), delay_buffer_.end(), 0.0f);
    write_pos_ = 0;
    phase_ = 0.0f;
}

} // namespace TopSpeed
