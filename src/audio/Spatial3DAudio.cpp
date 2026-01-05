#include "Spatial3DAudio.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vorbis/vorbisfile.h>

// OpenAL error checking
#define CHECK_AL_ERROR() { ALenum err = alGetError(); if (err != AL_NO_ERROR) { std::cerr << "[Audio] OpenAL Error: " << err << std::endl; } }

Spatial3DAudio::Spatial3DAudio() {
    // Initialize
}

Spatial3DAudio::~Spatial3DAudio() {
    Shutdown();
}

bool Spatial3DAudio::Initialize() {
    std::cout << "[3D Audio] Initializing spatial audio system..." << std::endl;

    // Open audio device
    al_device = alcOpenDevice(nullptr);
    if (!al_device) {
        std::cerr << "[3D Audio] Failed to open audio device" << std::endl;
        return false;
    }

    // Create audio context
    al_context = alcCreateContext(al_device, nullptr);
    if (!al_context) {
        std::cerr << "[3D Audio] Failed to create audio context" << std::endl;
        alcCloseDevice(al_device);
        return false;
    }

    // Make context current
    alcMakeContextCurrent(al_context);
    CHECK_AL_ERROR();

    // Set up listener (player position in world)
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
    alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    CHECK_AL_ERROR();

    // Set initial orientation (looking forward)
    ALfloat listenerOri[] = {0.0f, 0.0f, -1.0f,  // forward
                            0.0f, 1.0f, 0.0f};   // up
    alListenerfv(AL_ORIENTATION, listenerOri);
    CHECK_AL_ERROR();

    // Configure audio model
    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);  // Realistic distance falloff
    CHECK_AL_ERROR();

    // Set speed of sound and doppler
    alSpeedOfSound(speed_of_sound);
    alDopplerFactor(doppler_factor);
    CHECK_AL_ERROR();

    std::cout << "[3D Audio] System initialized successfully" << std::endl;
    return true;
}

void Spatial3DAudio::Shutdown() {
    std::cout << "[3D Audio] Shutting down spatial audio system..." << std::endl;

    // Stop and delete all sources
    for (auto idx : active_sources) {
        if (idx < sources.size()) {
            alDeleteSources(1, &sources[idx].al_source);
        }
    }
    active_sources.clear();

    // Delete all buffers
    for (auto& pair : loaded_sounds) {
        alDeleteBuffers(1, &pair.second);
    }
    loaded_sounds.clear();

    // Clean up OpenAL context
    if (al_context) {
        alcDestroyContext(al_context);
        al_context = nullptr;
    }

    if (al_device) {
        alcCloseDevice(al_device);
        al_device = nullptr;
    }

    std::cout << "[3D Audio] System shutdown complete" << std::endl;
}

// ========== LISTENER MANAGEMENT ==========

void Spatial3DAudio::SetListenerPosition(const glm::vec3& position,
                                        const glm::vec3& forward,
                                        const glm::vec3& up) {
    listener_position = position;
    listener_forward = glm::normalize(forward);
    listener_up = glm::normalize(up);

    // Update OpenAL listener position
    alListener3f(AL_POSITION, position.x, position.y, position.z);
    UpdateListenerOrientation();
    CHECK_AL_ERROR();
}

void Spatial3DAudio::SetListenerVelocity(const glm::vec3& velocity) {
    listener_velocity = velocity;
    alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
    CHECK_AL_ERROR();
}

void Spatial3DAudio::UpdateListenerOrientation() {
    ALfloat ori[] = {
        listener_forward.x, listener_forward.y, listener_forward.z,
        listener_up.x, listener_up.y, listener_up.z
    };
    alListenerfv(AL_ORIENTATION, ori);
}

// ========== 3D SOURCE MANAGEMENT ==========

int Spatial3DAudio::Create3DSource(const std::string& name) {
    // Find empty slot or create new
    int source_idx = -1;
    for (int i = 0; i < sources.size(); i++) {
        if (!sources[i].active) {
            source_idx = i;
            break;
        }
    }

    if (source_idx == -1) {
        source_idx = sources.size();
        sources.resize(sources.size() + 1);
    }

    SoundSource3D& source = sources[source_idx];

    // Create OpenAL source
    alGenSources(1, &source.al_source);
    CHECK_AL_ERROR();

    // Configure source for 3D audio
    alSourcef(source.al_source, AL_REFERENCE_DISTANCE, source.reference_distance);
    alSourcef(source.al_source, AL_MAX_DISTANCE, source.max_distance);
    alSourcef(source.al_source, AL_ROLLOFF_FACTOR, 1.0f);
    alSourcef(source.al_source, AL_PITCH, 1.0f);
    alSourcef(source.al_source, AL_GAIN, source.gain);
    CHECK_AL_ERROR();

    // Set position
    alSource3f(source.al_source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(source.al_source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    CHECK_AL_ERROR();

    source.active = true;
    source.name = name;
    active_sources.push_back(source_idx);

    std::cout << "[3D Audio] Created source " << source_idx << " (" << name << ")" << std::endl;
    return source_idx;
}

void Spatial3DAudio::Delete3DSource(int source_id) {
    if (source_id < 0 || source_id >= sources.size()) return;

    SoundSource3D& source = sources[source_id];
    if (source.al_source) {
        alDeleteSources(1, &source.al_source);
        CHECK_AL_ERROR();
    }

    source.active = false;

    // Remove from active list
    auto it = std::find(active_sources.begin(), active_sources.end(), source_id);
    if (it != active_sources.end()) {
        active_sources.erase(it);
    }
}

void Spatial3DAudio::Update3DSource(int source_id, const glm::vec3& position,
                                   const glm::vec3& velocity) {
    if (source_id < 0 || source_id >= sources.size()) return;

    SoundSource3D& source = sources[source_id];
    source.position = position;
    source.velocity = velocity;

    // Update OpenAL source position and velocity
    alSource3f(source.al_source, AL_POSITION, position.x, position.y, position.z);
    alSource3f(source.al_source, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
    CHECK_AL_ERROR();

    // Calculate distance-based gain
    float distance = CalculateDistance(position, listener_position);
    float gain = CalculateGainFromDistance(distance, source.reference_distance, 
                                          source.max_distance);
    alSourcef(source.al_source, AL_GAIN, gain);
    CHECK_AL_ERROR();
}

void Spatial3DAudio::Play3DSound(int source_id, const std::string& sound_name,
                                bool loop, float gain) {
    if (source_id < 0 || source_id >= sources.size()) return;

    // Find loaded sound
    auto it = loaded_sounds.find(sound_name);
    if (it == loaded_sounds.end()) {
        std::cerr << "[3D Audio] Sound not found: " << sound_name << std::endl;
        return;
    }

    SoundSource3D& source = sources[source_id];

    // Attach buffer to source
    alSourcei(source.al_source, AL_BUFFER, it->second);
    alSourcei(source.al_source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
    alSourcef(source.al_source, AL_GAIN, gain);
    CHECK_AL_ERROR();

    // Play sound
    alSourcePlay(source.al_source);
    CHECK_AL_ERROR();

    std::cout << "[3D Audio] Playing " << sound_name << " from source " << source_id << std::endl;
}

void Spatial3DAudio::Stop3DSource(int source_id) {
    if (source_id < 0 || source_id >= sources.size()) return;

    alSourceStop(sources[source_id].al_source);
    CHECK_AL_ERROR();
}

void Spatial3DAudio::Set3DSourceParameters(int source_id,
                                         float reference_distance,
                                         float max_distance) {
    if (source_id < 0 || source_id >= sources.size()) return;

    SoundSource3D& source = sources[source_id];
    source.reference_distance = reference_distance;
    source.max_distance = max_distance;

    alSourcef(source.al_source, AL_REFERENCE_DISTANCE, reference_distance);
    alSourcef(source.al_source, AL_MAX_DISTANCE, max_distance);
    CHECK_AL_ERROR();
}

// ========== GLOBAL SETTINGS ==========

void Spatial3DAudio::SetSpeedOfSound(float speed) {
    speed_of_sound = speed;
    alSpeedOfSound(speed);
    CHECK_AL_ERROR();
}

void Spatial3DAudio::SetDopplerFactor(float factor) {
    doppler_factor = factor;
    alDopplerFactor(factor);
    CHECK_AL_ERROR();
}

// ========== UPDATE ==========

void Spatial3DAudio::Update(float delta_time) {
    // Update positions of all active sources
    for (int idx : active_sources) {
        if (idx < sources.size() && sources[idx].active) {
            // Check if source is still playing
            ALint state;
            alGetSourcei(sources[idx].al_source, AL_SOURCE_STATE, &state);

            if (state != AL_PLAYING) {
                // Source finished, mark as inactive
                sources[idx].active = false;
            }
        }
    }

    // Remove inactive sources
    auto it = std::remove_if(active_sources.begin(), active_sources.end(),
                            [this](int idx) { return idx >= sources.size() || !sources[idx].active; });
    active_sources.erase(it, active_sources.end());
}

// ========== SOUND FILE MANAGEMENT ==========

bool Spatial3DAudio::LoadSound3D(const std::string& name, const std::string& filename) {
    // Check if already loaded
    if (loaded_sounds.find(name) != loaded_sounds.end()) {
        return true;
    }

    ALuint buffer = LoadOGGFile(filename);
    if (buffer == 0) {
        std::cerr << "[3D Audio] Failed to load sound: " << filename << std::endl;
        return false;
    }

    loaded_sounds[name] = buffer;
    std::cout << "[3D Audio] Loaded sound: " << name << std::endl;
    return true;
}

void Spatial3DAudio::UnloadSound3D(const std::string& name) {
    auto it = loaded_sounds.find(name);
    if (it != loaded_sounds.end()) {
        alDeleteBuffers(1, &it->second);
        loaded_sounds.erase(it);
        std::cout << "[3D Audio] Unloaded sound: " << name << std::endl;
    }
}

ALuint Spatial3DAudio::LoadOGGFile(const std::string& filename) {
    // TODO: Implement OGG loading using libvorbis
    // For now, return dummy buffer
    ALuint buffer;
    alGenBuffers(1, &buffer);
    return buffer;
}

// ========== HELPER FUNCTIONS ==========

float Spatial3DAudio::CalculateDistance(const glm::vec3& pos1, const glm::vec3& pos2) const {
    return glm::distance(pos1, pos2);
}

float Spatial3DAudio::CalculateGainFromDistance(float distance,
                                              float reference_distance,
                                              float max_distance) const {
    if (distance < 0.1f) distance = 0.1f;  // Avoid division by zero

    if (distance <= reference_distance) {
        return 1.0f;
    }

    if (distance >= max_distance) {
        return 0.0f;
    }

    // Inverse distance model: gain = reference_distance / distance
    float gain = reference_distance / distance;
    return std::min(gain, 1.0f);
}

float Spatial3DAudio::CalculatePanning(const glm::vec3& source_pos) const {
    // Calculate panning based on relative position
    // -1.0 = left, 0.0 = center, 1.0 = right

    glm::vec3 to_source = glm::normalize(source_pos - listener_position);

    // Right vector is perpendicular to forward
    glm::vec3 right = glm::cross(listener_forward, listener_up);
    right = glm::normalize(right);

    // Pan = dot product with right vector
    float pan = glm::dot(to_source, right);
    return glm::clamp(pan, -1.0f, 1.0f);
}

// ========== DEBUG ==========

std::string Spatial3DAudio::GetDebugInfo() const {
    std::string info = "[3D Audio Debug Info]\n";
    info += "Listener Position: (" + std::to_string(listener_position.x) + ", " +
            std::to_string(listener_position.y) + ", " +
            std::to_string(listener_position.z) + ")\n";
    info += "Active Sources: " + std::to_string(active_sources.size()) + "\n";
    info += "Loaded Sounds: " + std::to_string(loaded_sounds.size()) + "\n";
    info += "Speed of Sound: " + std::to_string(speed_of_sound) + "\n";
    info += "Doppler Factor: " + std::to_string(doppler_factor) + "\n";

    return info;
}
