#ifndef TOPSPEED_SETTINGS_DIALOG_H
#define TOPSPEED_SETTINGS_DIALOG_H

#include <string>
#include <vector>
#include <memory>

// Forward declarations
class AudioSystem;
class RenderSystem;
class UISystem;

/**
 * @class SettingsDialog
 * @brief In-game settings dialog for audio, graphics, and gameplay configuration
 * 
 * Provides a GUI for users to adjust:
 * - Audio levels (master, music, engine, SFX)
 * - Graphics settings (resolution, VSync)
 * - Gameplay settings (difficulty, transmission mode)
 * - Input settings (keyboard, gamepad)
 * 
 * No manual config file editing required!
 */
class SettingsDialog {
public:
    /**
     * @enum Tab
     * @brief Settings dialog tabs
     */
    enum class Tab {
        Audio = 0,      ///< Audio settings
        Graphics = 1,   ///< Graphics settings
        Gameplay = 2,   ///< Gameplay settings
        Input = 3,      ///< Input settings
        Count = 4       ///< Total tabs
    };

    // Constructor/Destructor
    SettingsDialog(AudioSystem* audio, RenderSystem* render, UISystem* ui);
    ~SettingsDialog();

    // Dialog control
    void Open();
    void Close();
    void Toggle();
    bool IsOpen() const { return is_open; }

    // Update and rendering
    void Update(float delta_time);
    void Render();

    // Input handling
    void HandleKeyInput(int key, int action);
    void HandleMouseInput(int button, int action, double x, double y);

private:
    // Dialog state
    bool is_open = false;
    Tab current_tab = Tab::Audio;
    float animation_progress = 0.0f;

    // System references
    AudioSystem* audio_system = nullptr;
    RenderSystem* render_system = nullptr;
    UISystem* ui_system = nullptr;

    // ========== AUDIO TAB ==========

    struct AudioTab {
        float master_volume = 0.8f;
        float music_volume = 0.6f;
        float engine_volume = 0.7f;
        float sfx_volume = 0.8f;

        bool audio_enabled = true;
        bool music_enabled = true;
        bool engine_enabled = true;
        bool sfx_enabled = true;

        int sample_rate = 44100;  // Hz
        int channels = 32;        // Max simultaneous sounds
        std::string device = "Default";

        // Volume slider control
        bool master_slider_active = false;
        bool music_slider_active = false;
        bool engine_slider_active = false;
        bool sfx_slider_active = false;
    } audio_tab;

    // ========== GRAPHICS TAB ==========

    struct GraphicsTab {
        int screen_width = 1920;
        int screen_height = 1080;
        bool vsync_enabled = true;
        bool anti_aliasing = true;
        bool fullscreen = false;

        // Resolution options
        std::vector<std::pair<int, int>> resolution_options = {
            {1280, 720},
            {1600, 900},
            {1920, 1080},
            {2560, 1440},
            {3840, 2160}
        };
        int resolution_index = 2;  // Default 1920x1080
    } graphics_tab;

    // ========== GAMEPLAY TAB ==========

    struct GameplayTab {
        enum class DifficultyMode {
            Easy = 0,
            Normal = 1,
            Hard = 2,
            Expert = 3
        };

        DifficultyMode difficulty = DifficultyMode::Normal;
        int num_laps = 3;
        int num_racers = 4;
        bool manual_transmission = false;
        bool enable_collisions = true;
        bool enable_damage = true;
        bool show_racing_line = false;

        // Difficulty names
        std::vector<std::string> difficulty_names = {
            "Easy", "Normal", "Hard", "Expert"
        };
    } gameplay_tab;

    // ========== INPUT TAB ==========

    struct InputTab {
        bool use_keyboard = true;
        bool use_gamepad = true;
        bool gamepad_detected = false;
        float deadzone = 0.15f;  // Gamepad stick deadzone

        // Input device info
        std::string keyboard_info = "Keyboard: Ready";
        std::string gamepad_info = "Gamepad: Not detected";
    } input_tab;

    // ========== UI RENDERING ==========

    // Render methods for each tab
    void RenderAudioTab();
    void RenderGraphicsTab();
    void RenderGameplayTab();
    void RenderInputTab();

    // Common UI elements
    void RenderSlider(const char* label, float& value, float min, float max, 
                     bool& active, float x, float y, float width);
    void RenderCheckbox(const char* label, bool& value, float x, float y);
    void RenderDropdown(const char* label, std::vector<std::string>& options, 
                       int& selected, float x, float y, float width);
    void RenderButton(const char* label, bool& pressed, float x, float y, 
                     float width, float height);

    // Special UI elements
    void RenderTestSoundButtons();
    void RenderVolumeVisualizer(float volume, float x, float y, float width);

    // ========== DIALOG BUTTONS ==========

    void RenderDialogButtons();
    void OnApply();
    void OnReset();
    void OnClose();

    // ========== AUDIO TESTING ==========

    void PlayTestSound(const std::string& sound_name);
    void PlayEngineSound(float speed_kmh);

    // ========== HELPER FUNCTIONS ==========

    bool IsMouseInRect(double mx, double my, float x, float y, float w, float h) const;
    void UpdateSlider(float& value, float min, float max, double mouse_x, float x, float width);
    void SaveSettings();
    void LoadSettings();
    void ApplySettings();
    void ResetToDefaults();

    // Layout helpers
    float GetTabHeight() const { return 500.0f; }
    float GetTabWidth() const { return 700.0f; }
    float GetDialogX() const { return 610.0f; }  // Centered at 1920x1080
    float GetDialogY() const { return 290.0f; }
};

#endif // TOPSPEED_SETTINGS_DIALOG_H
