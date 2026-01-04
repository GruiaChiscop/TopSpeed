#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <memory>

namespace TopSpeed {

/**
 * VorbisDecoder handles loading and decoding Ogg Vorbis files
 * Uses libvorbis for format handling
 */
class VorbisDecoder {
public:
    struct AudioData {
        std::vector<float> samples;  // Interleaved audio samples
        uint32_t sample_rate;
        uint32_t channels;
        uint64_t total_samples;
    };

    VorbisDecoder();
    ~VorbisDecoder();

    /**
     * Load and decode an Ogg Vorbis file
     * @param filename Path to .ogg file
     * @return Decoded audio data, or empty if failed
     */
    AudioData LoadFile(const std::string& filename);

    /**
     * Get last error message
     * @return Error description
     */
    const std::string& GetLastError() const { return last_error_; }

private:
    std::string last_error_;

    /**
     * Internal decoding implementation
     */
    AudioData DecodeVorbisFile(const std::string& filename);
};

} // namespace TopSpeed
