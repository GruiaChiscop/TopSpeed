#include "RaceTrack.h"
#include <algorithm>
#include <cmath>

RaceTrack::RaceTrack(const std::string& name) {
    definition_.name = name;
    definition_.type = TrackType::Normal;
    definition_.weather = WeatherType::Sunny;
    definition_.total_length = 0.0f;
}

RaceTrack::~RaceTrack() {
}

void RaceTrack::SetTrackDefinition(const TrackDefinition& definition) {
    definition_ = definition;

    // Calculate total length from segments
    float total = 0.0f;
    for (const auto& segment : definition_.road_segments) {
        total += segment.length;
    }
    definition_.total_length = total;
}

RaceTrack::RoadSegment RaceTrack::GetRoadSegmentAt(float distance) const {
    return FindSegmentAt(distance);
}

RaceTrack::RoadSegment RaceTrack::FindSegmentAt(float distance) const {
    // Handle wrapping (for repeating tracks)
    float wrapped_distance = std::fmod(distance, definition_.total_length);
    if (wrapped_distance < 0.0f) {
        wrapped_distance += definition_.total_length;
    }

    float current_pos = 0.0f;
    for (const auto& segment : definition_.road_segments) {
        if (wrapped_distance <= current_pos + segment.length) {
            return segment;
        }
        current_pos += segment.length;
    }

    // Return last segment if beyond track
    if (!definition_.road_segments.empty()) {
        return definition_.road_segments.back();
    }

    // Return default segment
    return RoadSegment{
        RoadType::Straight,
        SurfaceType::Asphalt,
        100.0f,
        -1.0f,
        1.0f,
        1.0f,
        "Road"
    };
}

RaceTrack::ObstacleSegment* RaceTrack::GetNearbyObstacle(float distance, float radius) {
    for (auto& obstacle : definition_.obstacles) {
        float dist_to_obstacle = std::abs(obstacle.position - distance);
        if (dist_to_obstacle <= radius) {
            return &obstacle;
        }
    }
    return nullptr;
}

bool RaceTrack::CheckObstacleCollision(float vehicle_distance, float vehicle_lane_pos, float vehicle_radius) {
    if (definition_.type != TrackType::Obstacle) {
        return false;  // No obstacles on normal tracks
    }

    for (auto& obstacle : definition_.obstacles) {
        // Check distance (along track)
        float dist_along_track = std::abs(obstacle.position - vehicle_distance);
        if (dist_along_track > (vehicle_radius + obstacle.radius)) {
            continue;  // Too far along track
        }

        // Check lateral position (simplified - obstacle at lane center)
        // Assume obstacles are centered in lane, vehicle has width based on radius
        float obstacle_lane = 0.0f;  // Center of lane
        float lateral_distance = std::abs(vehicle_lane_pos - obstacle_lane);

        if (lateral_distance <= (vehicle_radius + obstacle.radius * 0.5f)) {
            return true;  // Collision detected
        }
    }

    return false;
}

float RaceTrack::GetSpeedModifierAt(float distance) const {
    auto segment = FindSegmentAt(distance);

    // Base modifier from segment
    float modifier = segment.speed_modifier;

    // Apply weather effects
    switch (definition_.weather) {
        case WeatherType::Sunny:
            // No modifier
            break;
        case WeatherType::Rain:
            modifier *= 0.85f;  // 15% slower
            break;
        case WeatherType::Wind:
            modifier *= 0.90f;  // 10% slower
            break;
        case WeatherType::Storm:
            modifier *= 0.75f;  // 25% slower
            break;
    }

    // Apply surface effects
    switch (segment.surface) {
        case SurfaceType::Asphalt:
            // Best grip
            break;
        case SurfaceType::Gravel:
            modifier *= 0.80f;  // 20% slower
            break;
        case SurfaceType::Water:
            modifier *= 0.60f;  // 40% slower
            break;
        case SurfaceType::Sand:
            modifier *= 0.70f;  // 30% slower
            break;
        case SurfaceType::Snow:
            modifier *= 0.50f;  // 50% slower
            break;
    }

    // Cap between reasonable values
    return std::max(0.3f, std::min(1.5f, modifier));
}

std::string RaceTrack::GetRoadDescriptionAt(float distance) const {
    auto segment = FindSegmentAt(distance);
    return segment.description;
}

RaceTrack RaceTrack::CreateDefaultNormalTrack() {
    RaceTrack track("Default Circuit");

    TrackDefinition def;
    def.name = "Default Circuit";
    def.type = TrackType::Normal;
    def.weather = WeatherType::Sunny;
    def.total_length = 0.0f;  // Will be calculated

    // Create a simple circuit
    def.road_segments = {
        {RoadType::Straight, SurfaceType::Asphalt, 1000.0f, -1.0f, 1.0f, 1.0f, "Straight"},
        {RoadType::EasyRight, SurfaceType::Asphalt, 500.0f, -0.8f, 1.0f, 0.95f, "Easy right"},
        {RoadType::Straight, SurfaceType::Gravel, 800.0f, -1.0f, 1.0f, 0.85f, "Gravel section"},
        {RoadType::HardLeft, SurfaceType::Asphalt, 600.0f, -1.0f, 0.9f, 0.80f, "Hard left"},
        {RoadType::Straight, SurfaceType::Asphalt, 1200.0f, -1.0f, 1.0f, 1.0f, "Long straight"},
        {RoadType::EasyLeft, SurfaceType::Asphalt, 400.0f, -1.0f, 0.8f, 0.95f, "Easy left"},
    };

    def.obstacles = {};  // No obstacles on normal track

    track.SetTrackDefinition(def);
    return track;
}

RaceTrack RaceTrack::CreateDefaultObstacleTrack() {
    RaceTrack track("Hazard Course");

    TrackDefinition def;
    def.name = "Hazard Course";
    def.type = TrackType::Obstacle;
    def.weather = WeatherType::Rain;
    def.total_length = 0.0f;  // Will be calculated

    // Create track with obstacles
    def.road_segments = {
        {RoadType::Straight, SurfaceType::Asphalt, 500.0f, -1.0f, 1.0f, 1.0f, "Start"},
        {RoadType::Straight, SurfaceType::Water, 400.0f, -1.0f, 1.0f, 0.60f, "Wet section"},
        {RoadType::EasyRight, SurfaceType::Asphalt, 500.0f, -0.8f, 1.0f, 0.95f, "Right turn"},
        {RoadType::Straight, SurfaceType::Asphalt, 600.0f, -1.0f, 1.0f, 1.0f, "Clear road"},
        {RoadType::HardLeft, SurfaceType::Gravel, 500.0f, -1.0f, 0.9f, 0.80f, "Left turn gravel"},
        {RoadType::Straight, SurfaceType::Asphalt, 800.0f, -1.0f, 1.0f, 1.0f, "Final straight"},
    };

    // Add obstacles at various positions
    def.obstacles = {
        {520.0f, glm::vec3(520.0f, 0.0f, 0.0f), 30.0f, 15.0f, false, "pothole"},
        {950.0f, glm::vec3(950.0f, 0.0f, 0.0f), 50.0f, 30.0f, false, "barrier"},
        {1450.0f, glm::vec3(1450.0f, 0.0f, 0.0f), 25.0f, 5.0f, false, "puddle"},
        {1950.0f, glm::vec3(1950.0f, 0.0f, 0.0f), 40.0f, 20.0f, true, "trap"},
        {2550.0f, glm::vec3(2550.0f, 0.0f, 0.0f), 35.0f, 25.0f, false, "debris"},
    };

    track.SetTrackDefinition(def);
    return track;
}
