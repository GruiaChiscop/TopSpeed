#pragma once

#include <string>
#include <memory>

namespace TopSpeed {

/**
 * ScreenReaderManager provides accessibility features using universal-speech
 * This replaces audio menus with speech output for blind/visually impaired players
 */
class ScreenReaderManager {
public:
    enum class SpeechRate {
        Slow = -10,
        Normal = 0,
        Fast = 10
    };

    ScreenReaderManager();
    ~ScreenReaderManager();

    /**
     * Initialize screen reader subsystem
     * Attempts to connect to available screen reader (NVDA, JAWS, etc.)
     * @return true if successful or if universal-speech initialized
     */
    bool Initialize();

    /**
     * Speak text using screen reader or Text-to-Speech
     * @param text Text to speak
     * @param interrupt If true, stop current speech and speak new text
     */
    void Speak(const std::string& text, bool interrupt = true);

    /**
     * Speak text asynchronously (doesn't block)
     * @param text Text to speak
     */
    void SpeakAsync(const std::string& text);

    /**
     * Speak a menu option
     * @param menuText The menu option text
     * @param index Current position in menu
     * @param total Total menu items
     */
    void SpeakMenuOption(const std::string& menuText, int index, int total);

    /**
     * Announce game event
     * @param eventText Description of event (e.g., "Collision detected", "Lap completed")
     */
    void AnnounceEvent(const std::string& eventText);

    /**
     * Stop current speech
     */
    void Stop();

    /**
     * Set speech rate
     * @param rate Speech rate from SpeechRate enum
     */
    void SetSpeechRate(SpeechRate rate);

    /**
     * Set speech volume (0.0 = silent, 1.0 = normal)
     * @param volume Volume level
     */
    void SetVolume(float volume);

    /**
     * Check if screen reader is available
     * @return true if connected to screen reader
     */
    bool IsScreenReaderAvailable() const { return screen_reader_available_; }

    /**
     * Check if text-to-speech engine is available
     * @return true if TTS engine is initialized
     */
    bool IsTTSAvailable() const { return tts_available_; }

    /**
     * Enable/disable accessibility features
     * @param enabled true to enable
     */
    void SetEnabled(bool enabled) { enabled_ = enabled; }

    /**
     * Check if accessibility is enabled
     * @return true if enabled
     */
    bool IsEnabled() const { return enabled_; }

    /**
     * Shutdown screen reader manager
     */
    void Shutdown();

private:
    bool screen_reader_available_;  // Connected to system screen reader (NVDA, JAWS)
    bool tts_available_;            // Text-to-Speech engine available (universal-speech)
    bool enabled_;
    float volume_;
    SpeechRate speech_rate_;

    /**
     * Try to initialize system screen reader
     * @return true if successful
     */
    bool InitializeSystemScreenReader();

    /**
     * Initialize universal-speech TTS fallback
     * @return true if successful
     */
    bool InitializeUniversalSpeech();
};

} // namespace TopSpeed
