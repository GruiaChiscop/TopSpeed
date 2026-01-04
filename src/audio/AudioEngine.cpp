#include "AudioEngine.h"
#include <iostream>
#include <algorithm>

namespace TopSpeed {

AudioEngine::AudioEngine()
    : next_source_id_(1)
    , master_volume_(1.0f)
    , initialized_(false) {
}

AudioEngine::~AudioEngine() {
    Shutdown();
}

bool AudioEngine::Initialize() {
    if (initialized_) {
        return true;
    }

    // TODO: Initialize miniaudio device
    // ma_result result = ma_device_init(NULL, &device_config, &device);
    // if (result != MA_SUCCESS) {
    //     return false;
    // }
    // ma_device_start(&device);

    initialized_ = true;
    return true;
}

uint32_t AudioEngine::LoadAudio(const std::string& filename) {
    if (!initialized_) {
        std::cerr << "AudioEngine not initialized" << std::endl;
        return 0;
    }

    // TODO: Load audio file using miniaudio
    // ma_decoder decoder;
    // ma_result result = ma_decoder_init_file(filename.c_str(), NULL, &decoder);
    // if (result != MA_SUCCESS) {
    //     std::cerr << "Failed to load audio: " << filename << std::endl;
    //     return 0;
    // }

    uint32_t id = AllocateSourceId();
    AudioSource source;
    source.id = id;
    source.filename = filename;
    source.volume = 1.0f;
    source.looping = false;
    source.playing = false;

    sources_[id] = source;
    return id;
}

void AudioEngine::Play(uint32_t sourceId, bool looping) {
    auto it = sources_.find(sourceId);
    if (it != sources_.end()) {
        it->second.playing = true;
        it->second.looping = looping;
        // TODO: Start playback with miniaudio
    }
}

void AudioEngine::Stop(uint32_t sourceId) {
    auto it = sources_.find(sourceId);
    if (it != sources_.end()) {
        it->second.playing = false;
        // TODO: Stop playback with miniaudio
    }
}

void AudioEngine::Pause(uint32_t sourceId) {
    auto it = sources_.find(sourceId);
    if (it != sources_.end()) {
        it->second.playing = false;
        // TODO: Pause playback with miniaudio
    }
}

void AudioEngine::Resume(uint32_t sourceId) {
    auto it = sources_.find(sourceId);
    if (it != sources_.end()) {
        it->second.playing = true;
        // TODO: Resume playback with miniaudio
    }
}

void AudioEngine::SetVolume(uint32_t sourceId, float volume) {
    auto it = sources_.find(sourceId);
    if (it != sources_.end()) {
        it->second.volume = std::clamp(volume, 0.0f, 1.0f);
        // TODO: Update volume with miniaudio
    }
}

bool AudioEngine::IsPlaying(uint32_t sourceId) const {
    auto it = sources_.find(sourceId);
    if (it != sources_.end()) {
        return it->second.playing;
    }
    return false;
}

void AudioEngine::Unload(uint32_t sourceId) {
    auto it = sources_.find(sourceId);
    if (it != sources_.end()) {
        Stop(sourceId);
        // TODO: Cleanup decoder with miniaudio
        sources_.erase(it);
    }
}

void AudioEngine::UnloadAll() {
    std::vector<uint32_t> ids;
    for (const auto& pair : sources_) {
        ids.push_back(pair.first);
    }
    for (uint32_t id : ids) {
        Unload(id);
    }
}

void AudioEngine::SetMasterVolume(float volume) {
    master_volume_ = std::clamp(volume, 0.0f, 1.0f);
    // TODO: Update master volume with miniaudio
}

void AudioEngine::Update() {
    if (!initialized_) {
        return;
    }

    // TODO: Update audio playback state
    // Check for finished sources and clean up if not looping
}

void AudioEngine::Shutdown() {
    if (!initialized_) {
        return;
    }

    UnloadAll();

    // TODO: Uninitialize miniaudio device
    // ma_device_uninit(&device);

    initialized_ = false;
}

uint32_t AudioEngine::AllocateSourceId() {
    return next_source_id_++;
}

} // namespace TopSpeed
