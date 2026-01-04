#include "VorbisDecoder.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <stdexcept>

// Include Vorbis headers
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

namespace TopSpeed {

VorbisDecoder::VorbisDecoder() = default;
VorbisDecoder::~VorbisDecoder() = default;

VorbisDecoder::AudioData VorbisDecoder::LoadFile(const std::string& filename) {
    try {
        return DecodeVorbisFile(filename);
    } catch (const std::exception& e) {
        last_error_ = e.what();
        return AudioData();
    }
}

VorbisDecoder::AudioData VorbisDecoder::DecodeVorbisFile(const std::string& filename) {
    AudioData result;
    result.sample_rate = 0;
    result.channels = 0;
    result.total_samples = 0;

    // Open file
    FILE* file = nullptr;
#ifdef _WIN32
    if (fopen_s(&file, filename.c_str(), "rb") != 0 || !file) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
#else
    file = fopen(filename.c_str(), "rb");
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
#endif

    OggVorbis_File vf;
    std::memset(&vf, 0, sizeof(vf));

    // Initialize Vorbis decoder
    int ret = ov_open(file, &vf, nullptr, 0);
    if (ret != 0) {
        fclose(file);
        throw std::runtime_error("Failed to open Ogg file (error code: " + std::to_string(ret) + ")");
    }

    try {
        // Get file info
        vorbis_info* info = ov_info(&vf, -1);
        if (!info) {
            throw std::runtime_error("Failed to get vorbis info");
        }

        result.channels = info->channels;
        result.sample_rate = info->rate;
        result.total_samples = ov_pcm_total(&vf, -1);

        if (result.channels == 0 || result.sample_rate == 0) {
            throw std::runtime_error("Invalid audio format");
        }

        // Pre-allocate buffer
        size_t buffer_size = result.total_samples * result.channels;
        result.samples.reserve(buffer_size);

        // Decode audio
        const size_t BUFFER_SIZE = 4096;
        float** pcm_buffer = nullptr;
        int current_section = 0;

        while (true) {
            long samples_read = ov_read_float(&vf, &pcm_buffer, BUFFER_SIZE, &current_section);

            if (samples_read == 0) {
                // End of file
                break;
            } else if (samples_read < 0) {
                throw std::runtime_error("Error decoding Vorbis (error code: " + std::to_string(samples_read) + ")");
            }

            // Interleave samples
            for (long i = 0; i < samples_read; ++i) {
                for (int ch = 0; ch < result.channels; ++ch) {
                    result.samples.push_back(pcm_buffer[ch][i]);
                }
            }
        }

        std::cout << "Loaded Vorbis: " << filename
                  << " (" << result.channels << "ch, " << result.sample_rate << "Hz, "
                  << result.total_samples << " samples)" << std::endl;
    } catch (...) {
        ov_clear(&vf);
        throw;
    }

    ov_clear(&vf);
    return result;
}

} // namespace TopSpeed
