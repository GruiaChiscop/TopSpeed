#include "ScreenReaderManager.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <queue>
#include <mutex>

#ifdef _WIN32
    #include <windows.h>
    #include <sapi.h>
    #pragma comment(lib, "ole32.lib")
    #pragma comment(lib, "oleaut32.lib")
else
    #include <unistd.h>
#endif

namespace TopSpeed {

struct ScreenReaderImpl {
#ifdef _WIN32
    ISpVoice* pVoice = nullptr;
#endif
    std::queue<std::string> speech_queue;
    std::thread speech_thread;
    bool running = false;
    std::mutex queue_mutex;
};

ScreenReaderManager::ScreenReaderManager()
    : screen_reader_available_(false)
    , tts_available_(false)
    , enabled_(true)
    , volume_(1.0f)
    , speech_rate_(SpeechRate::Normal) {
}

ScreenReaderManager::~ScreenReaderManager() {
    Shutdown();
}

bool ScreenReaderManager::Initialize() {
    std::cout << "Initializing ScreenReaderManager..." << std::endl;

#ifdef _WIN32
    // Try Windows SAPI TTS
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hr)) {
        ISpVoice* pVoice = nullptr;
        hr = CoCreateInstance(CLSID_SpVoice, nullptr, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
        if (SUCCEEDED(hr) && pVoice) {
            tts_available_ = true;
            pVoice->Release();
            std::cout << "Windows SAPI TTS initialized" << std::endl;
        } else {
            CoUninitialize();
        }
    }
#endif

    // Try system screen reader detection
    if (InitializeSystemScreenReader()) {
        screen_reader_available_ = true;
        std::cout << "Connected to system screen reader" << std::endl;
    }

    // Fallback: Initialize universal-speech
    if (!tts_available_ && InitializeUniversalSpeech()) {
        tts_available_ = true;
        std::cout << "universal-speech TTS initialized" << std::endl;
    }

    if (!screen_reader_available_ && !tts_available_) {
        std::cerr << "Warning: No speech output available" << std::endl;
        return false;
    }

    // Start speech output thread
    // Announce initialization
    Speak("TopSpeed audio game initialized. Accessibility mode enabled.", false);
    return true;
}

bool ScreenReaderManager::InitializeSystemScreenReader() {
#ifdef _WIN32
    // Check if NVDA is running
    HWND nvda_window = FindWindowExA(nullptr, nullptr, "GUIMainWindow", nullptr);
    if (nvda_window != nullptr) {
        std::cout << "NVDA screen reader detected" << std::endl;
        return true;
    }

    // Check if JAWS is running
    HWND jaws_window = FindWindowExA(nullptr, nullptr, "JAWSHiliteWindow", nullptr);
    if (jaws_window != nullptr) {
        std::cout << "JAWS screen reader detected" << std::endl;
        return true;
    }
#endif
    return false;
}

bool ScreenReaderManager::InitializeUniversalSpeech() {
#ifdef _WIN32
    // Try to use Windows SAPI as fallback
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hr)) {
        ISpVoice* pVoice = nullptr;
        hr = CoCreateInstance(CLSID_SpVoice, nullptr, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
        if (SUCCEEDED(hr) && pVoice) {
            pVoice->Release();
            std::cout << "Windows SAPI initialized as speech engine" << std::endl;
            return true;
        }
        CoUninitialize();
    }
#else
    // On Linux/Mac, try festival or espeak
    #ifdef __APPLE__
        // macOS has built-in say command
        return true;
    #else
        // Linux check for festival or espeak
        int ret = system("which espeak > /dev/null 2>&1");
        if (ret == 0) {
            std::cout << "espeak speech engine detected" << std::endl;
            return true;
        }
    #endif
#endif
    return false;
}

void ScreenReaderManager::Speak(const std::string& text, bool interrupt) {
    if (!enabled_) {
        return;
    }

    // Always output to console for debugging
    std::cout << "[SPEECH] " << text << std::endl;

    // If screen reader available, send to it
    if (screen_reader_available_) {
        // NVDA/JAWS API would go here
        // For now, console output only
    }

    // If TTS available, queue speech
    if (tts_available_) {
#ifdef _WIN32
        HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        if (SUCCEEDED(hr)) {
            ISpVoice* pVoice = nullptr;
            hr = CoCreateInstance(CLSID_SpVoice, nullptr, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
            if (SUCCEEDED(hr) && pVoice) {
                // Set speech rate and volume
                pVoice->SetRate((long)speech_rate_);
                pVoice->SetVolume((USHORT)(volume_ * 100));

                // Convert to wide string for SAPI
                int size_needed = MultiByteToWideChar(CP_UTF8, 0, &text[0], (int)text.size(), NULL, 0);
                std::wstring wtext(size_needed, 0);
                MultiByteToWideChar(CP_UTF8, 0, &text[0], (int)text.size(), &wtext[0], size_needed);

                // Speak
                if (interrupt) {
                    pVoice->Speak(wtext.c_str(), SPF_PURGEBEFORESPEAK | SPF_ASYNC, nullptr);
                } else {
                    pVoice->Speak(wtext.c_str(), SPF_ASYNC, nullptr);
                }

                pVoice->Release();
            }
            CoUninitialize();
        }
#else
        // On macOS: use 'say' command
        #ifdef __APPLE__
            std::string cmd = "say \"" + text + "\" &";
            system(cmd.c_str());
        #else
            // On Linux: use espeak or festival
            int ret = system(("echo \"" + text + "\" | espeak 2>/dev/null &").c_str());
            if (ret != 0) {
                ret = system(("echo \"" + text + "\" | festival --tts 2>/dev/null &").c_str());
            }
        #endif
#endif
    }
}

void ScreenReaderManager::SpeakAsync(const std::string& text) {
    // For now, same as Speak (can be made truly async later)
    Speak(text, false);
}

void ScreenReaderManager::SpeakMenuOption(const std::string& menuText, int index, int total) {
    if (!enabled_) {
        return;
    }

    std::ostringstream oss;
    oss << "Menu item " << (index + 1) << " of " << total << ". " << menuText;
    Speak(oss.str(), true);
}

void ScreenReaderManager::AnnounceEvent(const std::string& eventText) {
    if (!enabled_) {
        return;
    }

    Speak(eventText, false);
}

void ScreenReaderManager::Stop() {
#ifdef _WIN32
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hr)) {
        ISpVoice* pVoice = nullptr;
        hr = CoCreateInstance(CLSID_SpVoice, nullptr, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
        if (SUCCEEDED(hr) && pVoice) {
            pVoice->Speak(nullptr, SPF_PURGEBEFORESPEAK, nullptr);
            pVoice->Release();
        }
        CoUninitialize();
    }
#endif
}

void ScreenReaderManager::SetSpeechRate(SpeechRate rate) {
    speech_rate_ = rate;
}

void ScreenReaderManager::SetVolume(float volume) {
    volume_ = volume;
}

void ScreenReaderManager::Shutdown() {
    std::cout << "ScreenReaderManager shutdown" << std::endl;
}

} // namespace TopSpeed
