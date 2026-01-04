#pragma once

#include <chrono>
#include <deque>
#include <cmath>

namespace TopSpeed {

/**
 * Game clock - precise timing for fixed timestep game loop
 * 
 * Features:
 * - Delta time calculation (time since last frame)
 * - Total elapsed time tracking
 * - Frame rate calculation (rolling average)
 * - Frame time statistics
 */
class GameClock {
public:
    GameClock();
    
    /**
     * Update clock (call once per frame)
     */
    void Update();
    
    /**
     * Get delta time since last frame (in seconds)
     */
    float GetDeltaTime() const { return delta_time_; }
    
    /**
     * Get total elapsed time since start (in seconds)
     */
    double GetTotalTime() const { return total_time_; }
    
    /**
     * Get current frame count
     */
    uint64_t GetFrameCount() const { return frame_count_; }
    
    /**
     * Get average frames per second (rolling average over last 60 frames)
     */
    float GetFrameRate() const { return frame_rate_; }
    
    /**
     * Get minimum frame time (lowest FPS) in last second
     */
    float GetMinFrameTime() const { return min_frame_time_; }
    
    /**
     * Get maximum frame time (highest latency) in last second
     */
    float GetMaxFrameTime() const { return max_frame_time_; }
    
    /**
     * Get average frame time in last second
     */
    float GetAverageFrameTime() const { return avg_frame_time_; }
    
    /**
     * Reset clock
     */
    void Reset();
    
    /**
     * Pause the clock (delta time will be zero)
     */
    void SetPaused(bool paused) { paused_ = paused; }
    bool IsPaused() const { return paused_; }
    
    /**
     * Get debug info string
     */
    std::string GetDebugInfo() const;

private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    
    TimePoint last_time_;
    TimePoint frame_time_start_;
    
    float delta_time_ = 0.0f;      // Time since last frame (seconds)
    double total_time_ = 0.0;      // Total elapsed time (seconds)
    uint64_t frame_count_ = 0;     // Total frames since start
    
    float frame_rate_ = 0.0f;      // FPS (rolling average)
    float min_frame_time_ = 0.0f;  // Min frame time in current window
    float max_frame_time_ = 0.0f;  // Max frame time in current window
    float avg_frame_time_ = 0.0f;  // Avg frame time in current window
    
    bool paused_ = false;
    
    // Frame time history (last 60 frames)
    std::deque<float> frame_times_;
    static constexpr size_t FRAME_HISTORY_SIZE = 60;
    
    // Update statistics
    void UpdateFrameStatistics();
};

} // namespace TopSpeed
