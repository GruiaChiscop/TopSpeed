#include "GameClock.h"
#include <iostream>
#include <numeric>
#include <sstream>

namespace TopSpeed {

GameClock::GameClock() {
    Reset();
}

void GameClock::Update() {
    if (paused_) {
        delta_time_ = 0.0f;
        return;
    }
    
    TimePoint now = Clock::now();
    
    // Calculate delta time on first call
    if (frame_count_ == 0) {
        delta_time_ = 0.016f;  // Default to ~60 FPS
    } else {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - last_time_);
        delta_time_ = duration.count() / 1000000.0f;
    }
    
    last_time_ = now;
    total_time_ += delta_time_;
    frame_count_++;
    
    // Update frame statistics
    UpdateFrameStatistics();
}

void GameClock::Reset() {
    last_time_ = Clock::now();
    frame_time_start_ = Clock::now();
    
    delta_time_ = 0.0f;
    total_time_ = 0.0;
    frame_count_ = 0;
    frame_rate_ = 0.0f;
    min_frame_time_ = 0.0f;
    max_frame_time_ = 0.0f;
    avg_frame_time_ = 0.0f;
    paused_ = false;
    
    frame_times_.clear();
}

std::string GameClock::GetDebugInfo() const {
    std::ostringstream oss;
    oss << "=== Game Clock ===\n";
    oss << "Frame: " << frame_count_ << "\n";
    oss << "Total Time: " << total_time_ << "s\n";
    oss << "Delta Time: " << delta_time_ << "s\n";
    oss << "FPS: " << frame_rate_ << "\n";
    oss << "Min Frame Time: " << (min_frame_time_ * 1000.0f) << "ms\n";
    oss << "Max Frame Time: " << (max_frame_time_ * 1000.0f) << "ms\n";
    oss << "Avg Frame Time: " << (avg_frame_time_ * 1000.0f) << "ms\n";
    oss << "Paused: " << (paused_ ? "yes" : "no");
    return oss.str();
}

void GameClock::UpdateFrameStatistics() {
    // Track frame time in history
    frame_times_.push_back(delta_time_);
    
    // Keep only recent frames
    if (frame_times_.size() > FRAME_HISTORY_SIZE) {
        frame_times_.pop_front();
    }
    
    // Calculate statistics from frame history
    if (!frame_times_.empty()) {
        // Find min and max
        min_frame_time_ = *std::min_element(frame_times_.begin(), frame_times_.end());
        max_frame_time_ = *std::max_element(frame_times_.begin(), frame_times_.end());
        
        // Calculate average
        float sum = std::accumulate(frame_times_.begin(), frame_times_.end(), 0.0f);
        avg_frame_time_ = sum / frame_times_.size();
        
        // Calculate rolling average FPS
        if (avg_frame_time_ > 0.0f) {
            frame_rate_ = 1.0f / avg_frame_time_;
        }
    }
}

} // namespace TopSpeed
