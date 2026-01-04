#include "AccessibilityManager.h"
#include <iostream>
#include <sstream>
#include <algorithm>

// Forward declare UniversalSpeech C API
// These would be included from UniversalSpeech.h when the library is available
extern "C" {
    // Core speech functions
    int US_initialize();
    void US_terminate();
    int US_isRunning();
    int US_speak(const char* text, int wait);
    int US_stop();
    int US_pause();
    int US_resume();
    int US_isSpeaking();

    // Parameter control
    void US_setRate(int rate);
    int US_getRate();
    void US_setPitch(int pitch);
    int US_getPitch();
    void US_setVolume(int volume);
    int US_getVolume();
    void US_setVoice(int voice);
    int US_getVoice();
    int US_getVoiceCount();
    const char* US_getVoiceName(int voice);

    // Engine management
    const char* US_getEngineNames();
    int US_setEngine(const char* engine);
    const char* US_getActiveEngine();

    // Language support
    int US_setLanguage(const char* language);
    const char* US_getLanguages();

    // Options
    void US_setUseSapi(int use);
    int US_getUseSapi();
}

class AccessibilityManager::Impl {
public:
    Impl() : initialized_(false), active_engine_(SpeechEngine::None) {}

    bool Initialize() {
        // Initialize UniversalSpeech
        if (US_initialize() != 0) {
            // Initialization failed
            std::cerr << "UniversalSpeech initialization failed" << std::endl;
            return false;
        }

        initialized_ = true;
        DetectEngine();
        return true;
    }

    void Shutdown() {
        if (initialized_) {
            US_terminate();
            initialized_ = false;
        }
    }

    bool IsInitialized() const {
        return initialized_ && (US_isRunning() != 0);
    }

    void DetectEngine() {
        const char* engine_name = US_getActiveEngine();
        if (!engine_name) {
            active_engine_ = SpeechEngine::None;
            return;
        }

        std::string engine(engine_name);
        if (engine.find("JAWS") != std::string::npos) {
            active_engine_ = SpeechEngine::JAWS;
        } else if (engine.find("NVDA") != std::string::npos) {
            active_engine_ = SpeechEngine::NVDA;
        } else if (engine.find("Window Eyes") != std::string::npos) {
            active_engine_ = SpeechEngine::WindowEyes;
        } else if (engine.find("System Access") != std::string::npos) {
            active_engine_ = SpeechEngine::SystemAccess;
        } else if (engine.find("Supernova") != std::string::npos) {
            active_engine_ = SpeechEngine::Supernova;
        } else if (engine.find("Cobra") != std::string::npos) {
            active_engine_ = SpeechEngine::Cobra;
        } else if (engine.find("SAPI") != std::string::npos) {
            active_engine_ = SpeechEngine::SAPI5;
        } else {
            active_engine_ = SpeechEngine::None;
        }
    }

    SpeechEngine GetActiveEngine() const {
        return active_engine_;
    }

    std::string GetActiveEngineName() const {
        const char* name = US_getActiveEngine();
        return name ? std::string(name) : "None";
    }

    bool IsSpeechAvailable() const {
        return IsInitialized() && active_engine_ != SpeechEngine::None;
    }

    bool Speak(const std::string& text, bool wait) {
        if (!IsInitialized() || !IsSpeechAvailable()) {
            return false;
        }
        return US_speak(text.c_str(), wait ? 1 : 0) == 0;
    }

    void Stop() {
        if (IsInitialized()) {
            US_stop();
        }
    }

    void Pause() {
        if (IsInitialized()) {
            US_pause();
        }
    }

    void Resume() {
        if (IsInitialized()) {
            US_resume();
        }
    }

    bool IsSpeaking() const {
        if (!IsInitialized()) return false;
        return US_isSpeaking() != 0;
    }

    void SetRate(int rate) {
        if (IsInitialized()) {
            // Clamp rate to valid range
            rate = std::max(-10, std::min(10, rate));
            US_setRate(rate);
        }
    }

    int GetRate() const {
        return IsInitialized() ? US_getRate() : 0;
    }

    void SetPitch(int pitch) {
        if (IsInitialized()) {
            pitch = std::max(-10, std::min(10, pitch));
            US_setPitch(pitch);
        }
    }

    int GetPitch() const {
        return IsInitialized() ? US_getPitch() : 0;
    }

    void SetVolume(int volume) {
        if (IsInitialized()) {
            volume = std::max(0, std::min(100, volume));
            US_setVolume(volume);
        }
    }

    int GetVolume() const {
        return IsInitialized() ? US_getVolume() : 100;
    }

    bool SetVoice(int voice_index) {
        if (!IsInitialized()) return false;
        if (voice_index < 0 || voice_index >= GetVoiceCount()) {
            return false;
        }
        US_setVoice(voice_index);
        return true;
    }

    int GetVoice() const {
        return IsInitialized() ? US_getVoice() : 0;
    }

    int GetVoiceCount() const {
        return IsInitialized() ? US_getVoiceCount() : 0;
    }

    std::vector<std::string> GetAvailableVoices() const {
        std::vector<std::string> voices;
        if (!IsInitialized()) return voices;

        int count = GetVoiceCount();
        for (int i = 0; i < count; ++i) {
            const char* name = US_getVoiceName(i);
            if (name) {
                voices.push_back(std::string(name));
            }
        }
        return voices;
    }

    bool SetLanguage(const std::string& language) {
        if (!IsInitialized()) return false;
        return US_setLanguage(language.c_str()) == 0;
    }

    std::vector<std::string> GetAvailableLanguages() const {
        std::vector<std::string> languages;
        if (!IsInitialized()) return languages;

        const char* lang_str = US_getLanguages();
        if (!lang_str) return languages;

        std::string langs(lang_str);
        std::stringstream ss(langs);
        std::string lang;
        while (std::getline(ss, lang, ',')) {
            if (!lang.empty()) {
                // Trim whitespace
                lang.erase(0, lang.find_first_not_of(" \t"));
                lang.erase(lang.find_last_not_of(" \t") + 1);
                languages.push_back(lang);
            }
        }
        return languages;
    }

    void SetUseSapi(bool use) {
        if (IsInitialized()) {
            US_setUseSapi(use ? 1 : 0);
        }
    }

    std::string GetDebugInfo() const {
        std::stringstream ss;
        ss << "AccessibilityManager Debug Info\n";
        ss << "- Initialized: " << (IsInitialized() ? "Yes" : "No") << "\n";
        ss << "- Speech Available: " << (IsSpeechAvailable() ? "Yes" : "No") << "\n";
        ss << "- Active Engine: " << GetActiveEngineName() << "\n";
        ss << "- Currently Speaking: " << (IsSpeaking() ? "Yes" : "No") << "\n";
        ss << "- Voice Count: " << GetVoiceCount() << "\n";
        ss << "- Current Voice: " << GetVoice() << "\n";
        ss << "- Rate: " << GetRate() << "\n";
        ss << "- Pitch: " << GetPitch() << "\n";
        ss << "- Volume: " << GetVolume() << "\n";
        return ss.str();
    }

private:
    bool initialized_;
    SpeechEngine active_engine_;
};

// AccessibilityManager implementation

AccessibilityManager::AccessibilityManager() : impl_(std::make_unique<Impl>()) {}

AccessibilityManager::~AccessibilityManager() {
    Shutdown();
}

bool AccessibilityManager::Initialize() {
    return impl_->Initialize();
}

void AccessibilityManager::Shutdown() {
    impl_->Shutdown();
}

bool AccessibilityManager::IsInitialized() const {
    return impl_->IsInitialized();
}

bool AccessibilityManager::IsSpeechAvailable() const {
    return impl_->IsSpeechAvailable();
}

AccessibilityManager::SpeechEngine AccessibilityManager::GetActiveEngine() const {
    return impl_->GetActiveEngine();
}

std::string AccessibilityManager::GetActiveEngineName() const {
    return impl_->GetActiveEngineName();
}

bool AccessibilityManager::Speak(const std::string& text, bool wait) {
    if (!enabled_) return false;
    return impl_->Speak(text, wait);
}

bool AccessibilityManager::SpeakTyped(const std::string& text, SpeechType type, bool wait) {
    // UniversalSpeech speaks all text the same way, but we can add prefixes
    std::string prefixed_text = text;
    if (type == SpeechType::Phonetic) {
        prefixed_text = "[phonetic] " + text;
    } else if (type == SpeechType::Abbreviation) {
        prefixed_text = "[abbr] " + text;
    }
    return Speak(prefixed_text, wait);
}

void AccessibilityManager::Stop() {
    impl_->Stop();
}

void AccessibilityManager::Pause() {
    impl_->Pause();
}

void AccessibilityManager::Resume() {
    impl_->Resume();
}

bool AccessibilityManager::IsSpeaking() const {
    return impl_->IsSpeaking();
}

void AccessibilityManager::SetRate(int rate) {
    current_rate_ = rate;
    impl_->SetRate(rate);
}

int AccessibilityManager::GetRate() const {
    return impl_->GetRate();
}

void AccessibilityManager::SetPitch(int pitch) {
    current_pitch_ = pitch;
    impl_->SetPitch(pitch);
}

int AccessibilityManager::GetPitch() const {
    return impl_->GetPitch();
}

void AccessibilityManager::SetVolume(int volume) {
    current_volume_ = std::max(0, std::min(100, volume));
    impl_->SetVolume(current_volume_);
}

int AccessibilityManager::GetVolume() const {
    return impl_->GetVolume();
}

bool AccessibilityManager::SetVoice(int voiceIndex) {
    if (impl_->SetVoice(voiceIndex)) {
        current_voice_ = voiceIndex;
        return true;
    }
    return false;
}

int AccessibilityManager::GetVoice() const {
    return impl_->GetVoice();
}

std::vector<std::string> AccessibilityManager::GetAvailableVoices() const {
    return impl_->GetAvailableVoices();
}

int AccessibilityManager::GetVoiceCount() const {
    return impl_->GetVoiceCount();
}

bool AccessibilityManager::SetLanguage(const std::string& language) {
    if (impl_->SetLanguage(language)) {
        current_language_ = language;
        return true;
    }
    return false;
}

std::vector<std::string> AccessibilityManager::GetAvailableLanguages() const {
    return impl_->GetAvailableLanguages();
}

bool AccessibilityManager::ForceEngine(SpeechEngine engine) {
    // Would require additional UniversalSpeech API functions
    // For now, we support engine detection but not forcing
    return false;
}

void AccessibilityManager::SetAllowSAPIFallback(bool allow) {
    allow_sapi_fallback_ = allow;
    impl_->SetUseSapi(allow);
}

bool AccessibilityManager::IsSAPIFallbackAllowed() const {
    return allow_sapi_fallback_;
}

std::vector<AccessibilityManager::SpeechEngine> AccessibilityManager::GetAvailableEngines() const {
    std::vector<SpeechEngine> engines;
    if (IsSpeechAvailable()) {
        engines.push_back(GetActiveEngine());
    }
    return engines;
}

std::vector<std::string> AccessibilityManager::GetAvailableEngineNames() const {
    std::vector<std::string> names;
    if (IsSpeechAvailable()) {
        names.push_back(GetActiveEngineName());
    }
    return names;
}

void AccessibilityManager::SetEnabled(bool enabled) {
    enabled_ = enabled;
}

bool AccessibilityManager::IsEnabled() const {
    return enabled_;
}

std::string AccessibilityManager::GetDebugInfo() const {
    return impl_->GetDebugInfo();
}

void AccessibilityManager::SetEventCallback(SpeechEventCallback callback) {
    // UniversalSpeech doesn't provide callbacks, but this interface supports them
    // Users could poll IsSpeaking() instead
}
