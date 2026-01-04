#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>

/**
 * @class AccessibilityManager
 * @brief Unified screen reader and speech synthesis interface using UniversalSpeech
 * 
 * Provides cross-platform screen reader support with automatic detection and fallback
 * to text-to-speech synthesis. Supports:
 * - JAWS for Windows
 * - NVDA
 * - Window Eyes
 * - System Access
 * - Supernova
 * - SAPI 5 (fallback)
 */
class AccessibilityManager {
public:
    // Speech synthesis engine types
    enum class SpeechEngine {
        None,                 // No speech available
        JAWS,                 // Freedom Scientific JAWS
        NVDA,                 // Open source NVDA
        WindowEyes,          // GW Micro Window Eyes
        SystemAccess,        // System Access SA
        Supernova,           // Dolphin Supernova
        Cobra,               // Cobra screen reader
        SAPI5                // Windows SAPI 5 (fallback)
    };

    // Speech output options
    enum class SpeechType {
        Text,               // Speak text
        Phonetic,          // Speak phonetically
        Abbreviation       // Speak as abbreviation
    };

    AccessibilityManager();
    ~AccessibilityManager();

    /**
     * Initialize accessibility system and detect available screen readers
     * @return true if initialization successful or screen reader detected
     */
    bool Initialize();

    /**
     * Shutdown accessibility system and release resources
     */
    void Shutdown();

    /**
     * Check if accessibility system is initialized
     */
    bool IsInitialized() const;

    /**
     * Check if any speech engine is available
     */
    bool IsSpeechAvailable() const;

    /**
     * Get currently active speech engine
     */
    SpeechEngine GetActiveEngine() const;

    /**
     * Get name of currently active speech engine
     */
    std::string GetActiveEngineName() const;

    /**
     * Speak text using active screen reader or TTS
     * @param text Text to speak
     * @param wait If true, block until speech finishes
     * @return true if speech initiated successfully
     */
    bool Speak(const std::string& text, bool wait = false);

    /**
     * Speak text with specified type (text, phonetic, abbreviation)
     */
    bool SpeakTyped(const std::string& text, SpeechType type, bool wait = false);

    /**
     * Stop current speech
     */
    void Stop();

    /**
     * Pause current speech
     */
    void Pause();

    /**
     * Resume paused speech
     */
    void Resume();

    /**
     * Check if currently speaking
     */
    bool IsSpeaking() const;

    /**
     * Set speech rate
     * @param rate Rate from -10 (slow) to +10 (fast), 0 = normal
     */
    void SetRate(int rate);

    /**
     * Get current speech rate
     */
    int GetRate() const;

    /**
     * Set speech pitch
     * @param pitch Pitch from -10 (low) to +10 (high), 0 = normal
     */
    void SetPitch(int pitch);

    /**
     * Get current speech pitch
     */
    int GetPitch() const;

    /**
     * Set speech volume
     * @param volume Volume from 0 (silent) to 100 (max)
     */
    void SetVolume(int volume);

    /**
     * Get current speech volume
     */
    int GetVolume() const;

    /**
     * Set voice (if supported)
     * @param voiceIndex Index of voice to use
     * @return true if voice set successfully
     */
    bool SetVoice(int voiceIndex);

    /**
     * Get current voice index
     */
    int GetVoice() const;

    /**
     * Get available voices
     * @return Vector of voice names
     */
    std::vector<std::string> GetAvailableVoices() const;

    /**
     * Get number of available voices
     */
    int GetVoiceCount() const;

    /**
     * Set language (if supported)
     * @param language Language code (e.g., "en-US", "fr-FR")
     * @return true if language set successfully
     */
    bool SetLanguage(const std::string& language);

    /**
     * Get available languages
     * @return Vector of language codes
     */
    std::vector<std::string> GetAvailableLanguages() const;

    /**
     * Force use of specific speech engine
     * @param engine Engine to force
     * @return true if engine is available
     */
    bool ForceEngine(SpeechEngine engine);

    /**
     * Allow fallback to SAPI 5 if other engines unavailable
     * @param allow If true, fallback to SAPI 5; if false, disable speech if no screen reader
     */
    void SetAllowSAPIFallback(bool allow);

    /**
     * Check if SAPI fallback is allowed
     */
    bool IsSAPIFallbackAllowed() const;

    /**
     * Get list of available speech engines on this system
     */
    std::vector<SpeechEngine> GetAvailableEngines() const;

    /**
     * Get list of available speech engine names on this system
     */
    std::vector<std::string> GetAvailableEngineNames() const;

    /**
     * Enable/disable accessibility features
     */
    void SetEnabled(bool enabled);

    /**
     * Check if accessibility features are enabled
     */
    bool IsEnabled() const;

    /**
     * Get debug information about accessibility system state
     */
    std::string GetDebugInfo() const;

    /**
     * Callback for speech events (start, stop, pause, resume)
     */
    using SpeechEventCallback = std::function<void(const std::string& event)>;
    void SetEventCallback(SpeechEventCallback callback);

private:
    // Internal implementation using UniversalSpeech
    class Impl;
    std::unique_ptr<Impl> impl_;

    bool enabled_ = true;
    int current_rate_ = 0;
    int current_pitch_ = 0;
    int current_volume_ = 100;
    int current_voice_ = 0;
    std::string current_language_;
    bool allow_sapi_fallback_ = true;
};
