#ifndef TOPSPEED_SPATIAL_3D_AUDIO_H
#define TOPSPEED_SPATIAL_3D_AUDIO_H

#include <AL/al.h>
#include <AL/alc.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

/**
 * @struct SoundSource3D
 * @brief Represents a 3D positioned sound source
 */
struct SoundSource3D {
    ALuint al_source = 0;           // OpenAL source ID
    ALuint al_buffer = 0;           // OpenAL buffer ID
    glm::vec3 position = {0, 0, 0}; // World position
    glm::vec3 velocity = {0, 0, 0}; // Velocity for doppler effect
    float gain = 1.0f;              // Volume (0.0 - 1.0)
    float reference_distance = 50.0f; // Distance where volume = 1.0
    float max_distance = 500.0f;    // Beyond this, audio is silent
    bool looping = false;           // Loop playback
    bool active = true;            // Is this source playing
    std::string name;              // Source name (for debugging)
};

/**
 * @class Spatial3DAudio
 * @brief 3D spatial audio system using OpenAL
 * 
 * Features:
 * - Positional audio (left/right/front/back based on world position)
 * - Distance attenuation (sounds get quieter with distance)
 * - Doppler effect (pitch changes when source moves toward/away)
 * - Listener positioning (player's ears in the world)
 * - Real-time 3D updates
 * 
 * How it works:
 * 1. Set listener position (player's car) with SetListenerPosition()
 * 2. Create 3D sources for each car/effect with Create3DSource()
 * 3. Update positions every frame with Update3DSource()
 * 4. Audio automatically pans left/right and adjusts volume based on distance
 */
class Spatial3DAudio {
public:
    // Constructor/Destructor
    Spatial3DAudio();
    ~Spatial3DAudio();

    // Initialization
    bool Initialize();
    void Shutdown();

    // Listener (Player) Management
    /**
     * @brief Set listener (player/camera) position in world
     * @param position World position of listener
     * @param forward Forward direction (where looking)
     * @param up Up direction
     */
    void SetListenerPosition(const glm::vec3& position, 
                            const glm::vec3& forward = {0, 0, -1},
                            const glm::vec3& up = {0, 1, 0});
    
    /**
     * @brief Set listener velocity for doppler effect
     * @param velocity Velocity vector
     */
    void SetListenerVelocity(const glm::vec3& velocity);

    // 3D Source Management
    /**
     * @brief Create a new 3D audio source
     * @param name Unique name for this source
     * @return Source ID, or -1 if failed
     */
    int Create3DSource(const std::string& name);

    /**
     * @brief Delete a 3D audio source
     * @param source_id Source ID returned from Create3DSource()
     */
    void Delete3DSource(int source_id);

    /**
     * @brief Update 3D source position
     * @param source_id Source ID
     * @param position New world position
     * @param velocity Velocity for doppler effect
     */
    void Update3DSource(int source_id, const glm::vec3& position, 
                       const glm::vec3& velocity = {0, 0, 0});

    /**
     * @brief Play sound from 3D source
     * @param source_id Source ID
     * @param sound_name Name of loaded sound
     * @param loop Should sound loop
     * @param gain Volume (0.0 - 1.0)
     */
    void Play3DSound(int source_id, const std::string& sound_name, 
                    bool loop = false, float gain = 1.0f);

    /**
     * @brief Stop 3D source
     * @param source_id Source ID
     */
    void Stop3DSource(int source_id);

    /**
     * @brief Set 3D source parameters
     * @param source_id Source ID
     * @param reference_distance Distance where gain = 1.0
     * @param max_distance Beyond this, gain = 0.0
     */
    void Set3DSourceParameters(int source_id, 
                              float reference_distance = 50.0f,
                              float max_distance = 500.0f);

    // Global Settings
    /**
     * @brief Set speed of sound (affects doppler effect)
     * @param speed Speed in units per second (default: 343.0 = speed of sound in m/s)
     */
    void SetSpeedOfSound(float speed);

    /**
     * @brief Set doppler factor (0 = no doppler, 1 = realistic, >1 = exaggerated)
     * @param factor Doppler factor
     */
    void SetDopplerFactor(float factor);

    // Update & Maintenance
    /**
     * @brief Update all 3D sources (call every frame)
     * @param delta_time Time since last frame
     */
    void Update(float delta_time);

    /**
     * @brief Get number of active sources
     * @return Count of active sources
     */
    int GetActiveSourceCount() const { return active_sources.size(); }

    // Sound File Management
    /**
     * @brief Load sound file for 3D playback
     * @param name Sound name identifier
     * @param filename Path to OGG file
     * @return true if loaded successfully
     */
    bool LoadSound3D(const std::string& name, const std::string& filename);

    /**
     * @brief Unload sound file
     * @param name Sound name identifier
     */
    void UnloadSound3D(const std::string& name);

    // Debug
    /**
     * @brief Get debug info about 3D audio system
     * @return Debug string
     */
    std::string GetDebugInfo() const;

private:
    // OpenAL context
    ALCdevice* al_device = nullptr;
    ALCcontext* al_context = nullptr;

    // Listener state
    glm::vec3 listener_position = {0, 0, 0};
    glm::vec3 listener_forward = {0, 0, -1};
    glm::vec3 listener_up = {0, 1, 0};
    glm::vec3 listener_velocity = {0, 0, 0};

    // 3D Sources
    std::vector<SoundSource3D> sources;
    std::vector<int> active_sources;  // Indices of active sources

    // Loaded sounds
    std::unordered_map<std::string, ALuint> loaded_sounds;

    // Global audio settings
    float speed_of_sound = 343.0f;    // Speed of sound (m/s)
    float doppler_factor = 1.0f;      // Doppler effect strength

    // Helper methods
    ALuint LoadOGGFile(const std::string& filename);
    void UpdateListenerOrientation();
    float CalculateDistance(const glm::vec3& pos1, const glm::vec3& pos2) const;
    float CalculateGainFromDistance(float distance, float reference_distance, 
                                   float max_distance) const;
    float CalculatePanning(const glm::vec3& source_pos) const;
};

#endif // TOPSPEED_SPATIAL_3D_AUDIO_H
