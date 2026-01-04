#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

/**
 * @class RaceTrack
 * @brief Represents a racing track with segments
 * 
 * Supports two track types:
 * 1. Normal - Standard racing tracks with surface variations (asphalt, gravel, etc.)
 * 2. Obstacle - Tracks with traps and obstacles to avoid
 */
class RaceTrack {
public:
    enum class TrackType {
        Normal,      // Standard racing track
        Obstacle     // Track with traps/obstacles to avoid
    };

    enum class RoadType {
        Straight,
        EasyLeft,
        Left,
        HardLeft,
        HairpinLeft,
        EasyRight,
        Right,
        HardRight,
        HairpinRight
    };

    enum class SurfaceType {
        Asphalt,
        Gravel,
        Water,
        Sand,
        Snow
    };

    enum class WeatherType {
        Sunny,
        Rain,
        Wind,
        Storm
    };

    struct RoadSegment {
        RoadType type;
        SurfaceType surface;
        float length;              // Distance of this segment
        float left_boundary;       // Left lane position
        float right_boundary;      // Right lane position
        float speed_modifier;      // How speed is affected (0.5-1.5)
        std::string description;   // For audio callout
    };

    struct ObstacleSegment {
        float position;            // Distance along track
        glm::vec3 location;       // 3D position
        float radius;             // Collision radius
        float damage;             // Damage if hit
        bool is_trap = false;     // Is a trap vs obstacle
        std::string type;         // "pothole", "barrier", "puddle", etc.
    };

    struct TrackDefinition {
        std::string name;
        TrackType type;
        WeatherType weather;
        float total_length;       // Total track length in units
        std::vector<RoadSegment> road_segments;
        std::vector<ObstacleSegment> obstacles;
    };

    RaceTrack(const std::string& name = "Track");
    ~RaceTrack();

    // Track setup
    void SetTrackDefinition(const TrackDefinition& definition);
    TrackDefinition GetTrackDefinition() const { return definition_; }

    // Track info
    TrackType GetTrackType() const { return definition_.type; }
    std::string GetTrackName() const { return definition_.name; }
    float GetTotalLength() const { return definition_.total_length; }
    WeatherType GetWeather() const { return definition_.weather; }

    // Road segment queries
    RoadSegment GetRoadSegmentAt(float distance) const;
    std::vector<RoadSegment> GetRoadSegments() const { return definition_.road_segments; }

    // Obstacle queries (Obstacle tracks only)
    std::vector<ObstacleSegment> GetObstacles() const { return definition_.obstacles; }
    ObstacleSegment* GetNearbyObstacle(float distance, float radius = 50.0f);
    bool CheckObstacleCollision(float vehicle_distance, float vehicle_lane_pos, float vehicle_radius);

    // Track evaluation
    float GetSpeedModifierAt(float distance) const;
    std::string GetRoadDescriptionAt(float distance) const;

    // Static track creation helpers
    static RaceTrack CreateDefaultNormalTrack();
    static RaceTrack CreateDefaultObstacleTrack();

private:
    TrackDefinition definition_;
    
    // Helper methods
    RoadSegment FindSegmentAt(float distance) const;
};
