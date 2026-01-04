#include "AudioEngine.h"
#include "VorbisDecoder.h"
#include <iostream>
#include <algorithm>
#include <cstring>
#include <thread>
#include <mutex>

// miniaudio implementation
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

namespace TopSpeed {

// Internal audio playback structure
struct AudioSourceData {
    uint32_t id;
    std::vector<float> samples;
    uint32_t sample_rate;
    uint32_t channels;
    size_t current_position = 0;
    bool looping = false;
    bool playing = false;
    float volume = 1.0f;
    std::string filename;
};

// Global miniaudio context (one per engine)
struct AudioEngineImpl {
    ma_context context;
    ma_device device;
    bool initialized = false;
};

AudioEngine::AudioEngine()
    : next_source_id_(1)
    , master_volume_(1.0f)
    , initialized_(false)
    , impl_(nullptr) {
}

AudioEngine::~AudioEngine() {
    Shutdown();
}

bool AudioEngine::Initialize() {
    if (initialized_) {
        return true;
    }

    try {
        impl_ = std::make_unique<AudioEngineImpl>();

        // Initialize miniaudio context
        ma_context_config context_config = ma_context_config_init();
        ma_result result = ma_context_init(nullptr, 0, &context_config, &impl_->context);
        if (result != MA_SUCCESS) {
            std::cerr << "Failed to initialize miniaudio context: " << ma_result_description(result) << std::endl;
            return false;
        }

        // Configure audio device
        ma_device_config device_config = ma_device_config_init(ma_device_type_playback);
        device_config.playback.format = ma_format_f32;
        device_config.playback.channels = 2;  // Stereo
        device_config.sampleRate = 44100;     // Common sample rate
        device_config.pUserData = this;
        device_config.dataCallback = [](ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
            static_cast<AudioEngine*>(pDevice->pUserData)->OnAudioData((float*)pOutput, frameCount);
        };

        // Initialize device
        result = ma_device_init(&impl_->context, &device_config, &impl_->device);
        if (result != MA_SUCCESS) {
            std::cerr << "Failed to initialize miniaudio device: " << ma_result_description(result) << std::endl;
            ma_context_uninit(&impl_->context);
            return false;
        }

        // Start playback
        result = ma_device_start(&impl_->device);
        if (result != MA_SUCCESS) {
            std::cerr << "Failed to start audio device: " << ma_result_description(result) << std::endl;
            ma_device_uninit(&impl_->device);
            ma_context_uninit(&impl_->context);
            return false;
        }

        initialized_ = true;
        std::cout << "AudioEngine initialized successfully" << std::endl;
        std::cout << "Device: " << impl_->device.playback.name
                  << " (" << impl_->device.playback.channels << "ch, "
                  << impl_->device.sampleRate << "Hz)" << std::endl;

        return true;
    } catch (const std::exception& e) {
        std::cerr << "AudioEngine initialization error: " << e.what() << std::endl;
        return false;
    }
}

uint32_t AudioEngine::LoadAudio(const std::string& filename) {
    if (!initialized_) {
        std::cerr << "AudioEngine not initialized" << std::endl;
        return 0;
    }

    // Load audio file
    VorbisDecoder decoder;
    auto audio_data = decoder.LoadFile(filename);

    if (audio_data.samples.empty()) {
        std::cerr << "Failed to load audio: " << filename
                  << " (" << decoder.GetLastError() << ")" << std::endl;
        return 0;
    }

    uint32_t id = AllocateSourceId();
    AudioSource source;
    source.id = id;
    source.filename = filename;
    source.volume = 1.0f;
    source.looping = false;
    source.playing = false;

    // Store audio data
    AudioSourceData source_data;
    source_data.id = id;
    source_data.samples = std::move(audio_data.samples);
    source_data.sample_rate = audio_data.sample_rate;
    source_data.channels = audio_data.channels;
    source_data.filename = filename;
    source_data.volume = 1.0f;
    source_data.looping = false;
    source_data.playing = false;
    source_data.current_position = 0;

    {
        std::lock_guard<std::mutex> lock(sources_mutex_);
        sources_[id] = source;
        sources_data_[id] = source_data;
    }

    std::cout << "Loaded audio source: " << id << " (" << filename << ")" << std::endl;
    return id;
}

void AudioEngine::Play(uint32_t sourceId, bool looping) {
    std::lock_guard<std::mutex> lock(sources_mutex_);

    auto it = sources_.find(sourceId);
    auto data_it = sources_data_.find(sourceId);

    if (it != sources_.end() && data_it != sources_data_.end()) {
        it->second.playing = true;
        it->second.looping = looping;
        data_it->second.playing = true;
        data_it->second.looping = looping;
        data_it->second.current_position = 0;
        std::cout << "Playing audio: " << sourceId << std::endl;
    }
}

void AudioEngine::Stop(uint32_t sourceId) {
    std::lock_guard<std::mutex> lock(sources_mutex_);

    auto it = sources_.find(sourceId);
    auto data_it = sources_data_.find(sourceId);

    if (it != sources_.end() && data_it != sources_data_.end()) {
        it->second.playing = false;
        data_it->second.playing = false;
        data_it->second.current_position = 0;
    }
}

void AudioEngine::Pause(uint32_t sourceId) {
    std::lock_guard<std::mutex> lock(sources_mutex_);

    auto it = sources_.find(sourceId);
    auto data_it = sources_data_.find(sourceId);

    if (it != sources_.end() && data_it != sources_data_.end()) {
        it->second.playing = false;
        data_it->second.playing = false;
    }
}

void AudioEngine::Resume(uint32_t sourceId) {
    std::lock_guard<std::mutex> lock(sources_mutex_);

    auto it = sources_.find(sourceId);
    auto data_it = sources_data_.find(sourceId);

    if (it != sources_.end() && data_it != sources_data_.end()) {
        it->second.playing = true;
        data_it->second.playing = true;
    }
}

void AudioEngine::SetVolume(uint32_t sourceId, float volume) {
    std::lock_guard<std::mutex> lock(sources_mutex_);

    auto it = sources_.find(sourceId);
    auto data_it = sources_data_.find(sourceId);

    if (it != sources_.end() && data_it != sources_data_.end()) {
        it->second.volume = std::clamp(volume, 0.0f, 1.0f);
        data_it->second.volume = it->second.volume;
    }
}

bool AudioEngine::IsPlaying(uint32_t sourceId) const {
    std::lock_guard<std::mutex> lock(sources_mutex_);

    auto it = sources_.find(sourceId);
    if (it != sources_.end()) {
        return it->second.playing;
    }
    return false;
}

void AudioEngine::Unload(uint32_t sourceId) {
    std::lock_guard<std::mutex> lock(sources_mutex_);

    auto it = sources_.find(sourceId);
    auto data_it = sources_data_.find(sourceId);

    if (it != sources_.end()) {
        it->second.playing = false;
        sources_.erase(it);
    }

    if (data_it != sources_data_.end()) {
        sources_data_.erase(data_it);
    }
}

void AudioEngine::UnloadAll() {
    std::vector<uint32_t> ids;
    {
        std::lock_guard<std::mutex> lock(sources_mutex_);
        for (const auto& pair : sources_) {
            ids.push_back(pair.first);
        }
    }

    for (uint32_t id : ids) {
        Unload(id);
    }
}

void AudioEngine::SetMasterVolume(float volume) {
    master_volume_ = std::clamp(volume, 0.0f, 1.0f);
    if (initialized_ && impl_) {
        ma_device_set_master_volume(&impl_->device, master_volume_);
    }
}

void AudioEngine::Update() {
    // Called from game loop - can be used for cleanup of finished sources
    std::lock_guard<std::mutex> lock(sources_mutex_);

    for (auto& pair : sources_data_) {
        auto& source = pair.second;
        if (source.playing && source.current_position >= source.samples.size() / source.channels) {
            if (source.looping) {
                source.current_position = 0;
            } else {
                source.playing = false;
            }
        }
    }
}

void AudioEngine::Shutdown() {
    if (!initialized_) {
        return;
    }

    UnloadAll();

    if (impl_) {
        ma_device_uninit(&impl_->device);
        ma_context_uninit(&impl_->context);
        impl_.reset();
    }

    initialized_ = false;
    std::cout << "AudioEngine shutdown" << std::endl;
}

uint32_t AudioEngine::AllocateSourceId() {
    return next_source_id_++;
}

void AudioEngine::OnAudioData(float* pOutput, uint32_t frameCount) {
    // Clear output buffer
    std::memset(pOutput, 0, frameCount * sizeof(float) * 2);  // Stereo

    std::lock_guard<std::mutex> lock(sources_mutex_);

    // Mix all active sources
    for (auto& pair : sources_data_) {
        auto& source = pair.second;

        if (!source.playing || source.samples.empty()) {
            continue;
        }

        float combined_volume = source.volume * master_volume_;
        size_t samples_remaining = source.samples.size() / source.channels;

        for (uint32_t frame = 0; frame < frameCount; ++frame) {
            if (source.current_position >= samples_remaining) {
                if (source.looping) {
                    source.current_position = 0;
                } else {
                    source.playing = false;
                    break;
                }
            }

            size_t sample_idx = source.current_position * source.channels;
            for (uint32_t ch = 0; ch < source.channels && ch < 2; ++ch) {
                pOutput[frame * 2 + ch] += source.samples[sample_idx + ch] * combined_volume;
            }

            source.current_position++;
        }
    }
}

} // namespace TopSpeed
