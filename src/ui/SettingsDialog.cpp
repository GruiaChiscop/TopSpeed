#include "SettingsDialog.h"
#include "../audio/AudioSystem.h"
#include "../graphics/RenderSystem.h"
#include "UISystem.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

SettingsDialog::SettingsDialog(AudioSystem* audio, RenderSystem* render, UISystem* ui)
    : audio_system(audio), render_system(render), ui_system(ui) {
    LoadSettings();
}

SettingsDialog::~SettingsDialog() {
    SaveSettings();
}

// ========== DIALOG CONTROL ==========

void SettingsDialog::Open() {
    is_open = true;
    animation_progress = 0.0f;
}

void SettingsDialog::Close() {
    is_open = false;
    animation_progress = 0.0f;
}

void SettingsDialog::Toggle() {
    if (is_open) {
        Close();
    } else {
        Open();
    }
}

// ========== UPDATE & RENDERING ==========

void SettingsDialog::Update(float delta_time) {
    if (!is_open) {
        return;
    }

    // Animate dialog appearance
    if (animation_progress < 1.0f) {
        animation_progress += delta_time * 2.0f;  // 0.5 second animation
        animation_progress = std::min(animation_progress, 1.0f);
    }
}

void SettingsDialog::Render() {
    if (!is_open || !ui_system) {
        return;
    }

    if (animation_progress <= 0.0f) {
        return;
    }

    float dialog_x = GetDialogX();
    float dialog_y = GetDialogY();
    float dialog_w = GetTabWidth();
    float dialog_h = GetTabHeight() + 100.0f;  // Extra space for buttons

    // Draw semi-transparent background overlay
    ui_system->DrawRect(0, 0, 1920, 1080, {0, 0, 0, 128}, true);

    // Draw dialog window
    ui_system->DrawRect(dialog_x - 5, dialog_y - 5, dialog_w + 10, dialog_h + 10, 
                       {255, 255, 255, 255}, false);
    ui_system->DrawRect(dialog_x, dialog_y, dialog_w, dialog_h, 
                       {50, 50, 50, 255}, true);

    // Draw title
    ui_system->DrawText("SETTINGS", dialog_x + dialog_w / 2 - 40, dialog_y + 10, 
                       {255, 255, 255, 255}, 2.0f);

    // Draw tab buttons
    float tab_width = dialog_w / 4;
    const char* tab_names[] = {"Audio", "Graphics", "Gameplay", "Input"};

    for (int i = 0; i < 4; i++) {
        float tab_x = dialog_x + (i * tab_width);
        float tab_y = dialog_y + 35;

        // Highlight active tab
        if (static_cast<int>(current_tab) == i) {
            ui_system->DrawRect(tab_x + 2, tab_y + 2, tab_width - 4, 25, 
                               {100, 150, 200, 255}, true);
        } else {
            ui_system->DrawRect(tab_x + 2, tab_y + 2, tab_width - 4, 25, 
                               {70, 70, 70, 255}, true);
        }

        ui_system->DrawText(tab_names[i], tab_x + tab_width / 2 - 15, tab_y + 5, 
                           {255, 255, 255, 255}, 1.0f);
    }

    // Draw tab content
    float content_y = dialog_y + 70;
    ui_system->PushScissor(dialog_x + 5, content_y + 5, dialog_w - 10, 
                          GetTabHeight() - 10);

    switch (current_tab) {
        case Tab::Audio:
            RenderAudioTab();
            break;
        case Tab::Graphics:
            RenderGraphicsTab();
            break;
        case Tab::Gameplay:
            RenderGameplayTab();
            break;
        case Tab::Input:
            RenderInputTab();
            break;
        default:
            break;
    }

    ui_system->PopScissor();

    // Draw dialog buttons
    RenderDialogButtons();
}

// ========== AUDIO TAB RENDERING ==========

void SettingsDialog::RenderAudioTab() {
    float x = GetDialogX() + 20;
    float y = GetDialogY() + 80;
    float spacing = 35;

    // Title
    ui_system->DrawText("Audio Settings", x, y, {255, 255, 255, 255}, 1.5f);
    y += 30;

    // Master Volume Slider
    ui_system->DrawText("Master Volume:", x, y, {200, 200, 200, 255}, 1.0f);
    RenderSlider("master", audio_tab.master_volume, 0.0f, 1.0f,
                audio_tab.master_slider_active, x + 150, y - 2, 150);
    y += spacing;

    // Music Volume Slider
    ui_system->DrawText("Music Volume:", x, y, {200, 200, 200, 255}, 1.0f);
    RenderSlider("music", audio_tab.music_volume, 0.0f, 1.0f,
                audio_tab.music_slider_active, x + 150, y - 2, 150);
    y += spacing;

    // Engine Volume Slider
    ui_system->DrawText("Engine Volume:", x, y, {200, 200, 200, 255}, 1.0f);
    RenderSlider("engine", audio_tab.engine_volume, 0.0f, 1.0f,
                audio_tab.engine_slider_active, x + 150, y - 2, 150);
    y += spacing;

    // SFX Volume Slider
    ui_system->DrawText("SFX Volume:", x, y, {200, 200, 200, 255}, 1.0f);
    RenderSlider("sfx", audio_tab.sfx_volume, 0.0f, 1.0f,
                audio_tab.sfx_slider_active, x + 150, y - 2, 150);
    y += spacing + 10;

    // Checkboxes
    RenderCheckbox("Audio Enabled", audio_tab.audio_enabled, x, y);
    y += spacing;
    RenderCheckbox("Music Enabled", audio_tab.music_enabled, x, y);
    y += spacing;
    RenderCheckbox("Engine Sounds", audio_tab.engine_enabled, x, y);
    y += spacing;
    RenderCheckbox("Sound Effects", audio_tab.sfx_enabled, x, y);
    y += spacing + 10;

    // Device selection
    ui_system->DrawText("Audio Device:", x, y, {200, 200, 200, 255}, 1.0f);
    ui_system->DrawText(audio_tab.device.c_str(), x + 150, y, 
                       {100, 200, 100, 255}, 1.0f);
    y += spacing + 10;

    // Test sounds section
    ui_system->DrawText("Test Sounds:", x, y, {255, 255, 100, 255}, 1.0f);
    y += spacing - 5;
    RenderTestSoundButtons();
}

// ========== GRAPHICS TAB RENDERING ==========

void SettingsDialog::RenderGraphicsTab() {
    float x = GetDialogX() + 20;
    float y = GetDialogY() + 80;
    float spacing = 35;

    // Title
    ui_system->DrawText("Graphics Settings", x, y, {255, 255, 255, 255}, 1.5f);
    y += 30;

    // Resolution
    ui_system->DrawText("Resolution:", x, y, {200, 200, 200, 255}, 1.0f);
    std::string res_str = std::to_string(graphics_tab.screen_width) + "x" + 
                         std::to_string(graphics_tab.screen_height);
    ui_system->DrawText(res_str.c_str(), x + 150, y, {100, 200, 100, 255}, 1.0f);
    y += spacing;

    // VSync
    RenderCheckbox("VSync", graphics_tab.vsync_enabled, x, y);
    y += spacing;

    // Anti-aliasing
    RenderCheckbox("Anti-Aliasing", graphics_tab.anti_aliasing, x, y);
    y += spacing;

    // Fullscreen
    RenderCheckbox("Fullscreen", graphics_tab.fullscreen, x, y);
    y += spacing + 10;

    // Display info
    ui_system->DrawText("Current FPS: 60", x, y, {150, 200, 150, 255}, 1.0f);
    y += spacing;
    ui_system->DrawText("Frame Time: 16.67ms", x, y, {150, 200, 150, 255}, 1.0f);
}

// ========== GAMEPLAY TAB RENDERING ==========

void SettingsDialog::RenderGameplayTab() {
    float x = GetDialogX() + 20;
    float y = GetDialogY() + 80;
    float spacing = 35;

    // Title
    ui_system->DrawText("Gameplay Settings", x, y, {255, 255, 255, 255}, 1.5f);
    y += 30;

    // Difficulty
    ui_system->DrawText("Difficulty:", x, y, {200, 200, 200, 255}, 1.0f);
    ui_system->DrawText(gameplay_tab.difficulty_names[
        static_cast<int>(gameplay_tab.difficulty)].c_str(), 
        x + 150, y, {100, 200, 100, 255}, 1.0f);
    y += spacing;

    // Number of laps
    ui_system->DrawText("Laps:", x, y, {200, 200, 200, 255}, 1.0f);
    ui_system->DrawText(std::to_string(gameplay_tab.num_laps).c_str(), 
                       x + 150, y, {100, 200, 100, 255}, 1.0f);
    y += spacing;

    // Number of racers
    ui_system->DrawText("Racers:", x, y, {200, 200, 200, 255}, 1.0f);
    ui_system->DrawText(std::to_string(gameplay_tab.num_racers).c_str(), 
                       x + 150, y, {100, 200, 100, 255}, 1.0f);
    y += spacing + 10;

    // Checkboxes
    RenderCheckbox("Manual Transmission", gameplay_tab.manual_transmission, x, y);
    y += spacing;
    RenderCheckbox("Enable Collisions", gameplay_tab.enable_collisions, x, y);
    y += spacing;
    RenderCheckbox("Enable Damage", gameplay_tab.enable_damage, x, y);
    y += spacing;
    RenderCheckbox("Show Racing Line", gameplay_tab.show_racing_line, x, y);
}

// ========== INPUT TAB RENDERING ==========

void SettingsDialog::RenderInputTab() {
    float x = GetDialogX() + 20;
    float y = GetDialogY() + 80;
    float spacing = 35;

    // Title
    ui_system->DrawText("Input Settings", x, y, {255, 255, 255, 255}, 1.5f);
    y += 30;

    // Input devices
    RenderCheckbox("Keyboard Enabled", input_tab.use_keyboard, x, y);
    y += spacing;
    ui_system->DrawText(input_tab.keyboard_info.c_str(), x + 20, y - 5, 
                       {100, 200, 100, 255}, 0.8f);
    y += spacing;

    RenderCheckbox("Gamepad Enabled", input_tab.use_gamepad, x, y);
    y += spacing;
    ui_system->DrawText(input_tab.gamepad_info.c_str(), x + 20, y - 5, 
                       input_tab.gamepad_detected ? 
                       std::array<uint8_t, 4>{100, 200, 100, 255} :
                       std::array<uint8_t, 4>{200, 100, 100, 255}, 0.8f);
    y += spacing + 10;

    // Deadzone
    ui_system->DrawText("Gamepad Deadzone:", x, y, {200, 200, 200, 255}, 1.0f);
    ui_system->DrawText(std::to_string(static_cast<int>(input_tab.deadzone * 100)) + "%",
                       x + 150, y, {100, 200, 100, 255}, 1.0f);
}

// ========== UI HELPER METHODS ==========

void SettingsDialog::RenderSlider(const char* label, float& value, float min, float max,
                                 bool& active, float x, float y, float width) {
    // Draw slider background
    ui_system->DrawRect(x, y + 5, width, 8, {50, 50, 50, 255}, true);

    // Draw filled portion
    float filled = (value - min) / (max - min);
    ui_system->DrawRect(x, y + 5, width * filled, 8, {100, 150, 200, 255}, true);

    // Draw slider handle
    float handle_x = x + (width * filled) - 4;
    ui_system->DrawRect(handle_x, y - 2, 8, 18, {150, 200, 255, 255}, true);

    // Display percentage
    std::string percentage = std::to_string(static_cast<int>(value * 100)) + "%";
    ui_system->DrawText(percentage.c_str(), x + width + 10, y + 2, 
                       {200, 200, 200, 255}, 0.8f);
}

void SettingsDialog::RenderCheckbox(const char* label, bool& value, float x, float y) {
    // Draw checkbox
    ui_system->DrawRect(x, y, 16, 16, {50, 50, 50, 255}, true);
    ui_system->DrawRect(x, y, 16, 16, {150, 150, 150, 255}, false);

    // Draw checkmark if enabled
    if (value) {
        ui_system->DrawText("\u2713", x + 2, y - 2, {100, 200, 100, 255}, 1.0f);
    }

    // Draw label
    ui_system->DrawText(label, x + 25, y + 2, {200, 200, 200, 255}, 1.0f);
}

void SettingsDialog::RenderTestSoundButtons() {
    float x = GetDialogX() + 20;
    float y = GetDialogY() + 310;

    // Test buttons
    bool pressed = false;
    RenderButton("[Idle]", pressed, x, y, 60, 25);
    if (pressed) PlayEngineSound(5.0f);

    RenderButton("[Low]", pressed, x + 70, y, 60, 25);
    if (pressed) PlayEngineSound(30.0f);

    RenderButton("[Mid]", pressed, x + 140, y, 60, 25);
    if (pressed) PlayEngineSound(100.0f);

    RenderButton("[High]", pressed, x + 210, y, 60, 25);
    if (pressed) PlayEngineSound(200.0f);

    RenderButton("[Shift]", pressed, x + 280, y, 60, 25);
    if (pressed) PlayTestSound("shift_up");
}

void SettingsDialog::RenderButton(const char* label, bool& pressed, float x, float y,
                                 float width, float height) {
    // Draw button
    ui_system->DrawRect(x, y, width, height, {70, 70, 70, 255}, true);
    ui_system->DrawRect(x, y, width, height, {150, 150, 150, 255}, false);

    // Center text
    float text_x = x + width / 2 - (label != nullptr ? strlen(label) * 2 : 0);
    float text_y = y + height / 2 - 5;
    ui_system->DrawText(label, text_x, text_y, {255, 255, 255, 255}, 0.8f);

    pressed = false;  // TODO: Handle mouse click detection
}

void SettingsDialog::RenderDialogButtons() {
    float dialog_x = GetDialogX();
    float dialog_y = GetDialogY() + GetTabHeight() + 80;
    float button_width = 100;
    float button_height = 25;
    float button_spacing = 20;

    float x = dialog_x + GetTabWidth() / 2 - (button_width * 3 + button_spacing * 2) / 2;

    // Apply button
    bool apply_pressed = false;
    RenderButton("[APPLY]", apply_pressed, x, dialog_y, button_width, button_height);
    if (apply_pressed) OnApply();

    // Reset button
    bool reset_pressed = false;
    x += button_width + button_spacing;
    RenderButton("[RESET]", reset_pressed, x, dialog_y, button_width, button_height);
    if (reset_pressed) OnReset();

    // Close button
    bool close_pressed = false;
    x += button_width + button_spacing;
    RenderButton("[CLOSE]", close_pressed, x, dialog_y, button_width, button_height);
    if (close_pressed) OnClose();
}

// ========== INPUT HANDLING ==========

void SettingsDialog::HandleKeyInput(int key, int action) {
    if (!is_open) return;

    // Tab switching with arrow keys
    if (action == 1) {  // Key pressed
        if (key == 262) {  // Right arrow
            int current = static_cast<int>(current_tab);
            current = (current + 1) % static_cast<int>(Tab::Count);
            current_tab = static_cast<Tab>(current);
        } else if (key == 263) {  // Left arrow
            int current = static_cast<int>(current_tab);
            current = (current - 1 + static_cast<int>(Tab::Count)) % static_cast<int>(Tab::Count);
            current_tab = static_cast<Tab>(current);
        } else if (key == 256) {  // ESC
            Close();
        }
    }
}

void SettingsDialog::HandleMouseInput(int button, int action, double x, double y) {
    if (!is_open) return;
    
    // TODO: Implement mouse click handling for buttons and sliders
}

// ========== SETTINGS MANAGEMENT ==========

void SettingsDialog::SaveSettings() {
    std::ofstream file("config/settings.ini");
    if (!file.is_open()) {
        std::cerr << "[Settings] Failed to save settings" << std::endl;
        return;
    }

    // Audio settings
    file << "[Audio]\n";
    file << "master_volume=" << audio_tab.master_volume << "\n";
    file << "music_volume=" << audio_tab.music_volume << "\n";
    file << "engine_volume=" << audio_tab.engine_volume << "\n";
    file << "sfx_volume=" << audio_tab.sfx_volume << "\n";
    file << "audio_enabled=" << (audio_tab.audio_enabled ? 1 : 0) << "\n";

    // Graphics settings
    file << "[Graphics]\n";
    file << "screen_width=" << graphics_tab.screen_width << "\n";
    file << "screen_height=" << graphics_tab.screen_height << "\n";
    file << "vsync=" << (graphics_tab.vsync_enabled ? 1 : 0) << "\n";
    file << "anti_aliasing=" << (graphics_tab.anti_aliasing ? 1 : 0) << "\n";

    // Gameplay settings
    file << "[Gameplay]\n";
    file << "difficulty=" << static_cast<int>(gameplay_tab.difficulty) << "\n";
    file << "num_laps=" << gameplay_tab.num_laps << "\n";
    file << "num_racers=" << gameplay_tab.num_racers << "\n";
    file << "manual_transmission=" << (gameplay_tab.manual_transmission ? 1 : 0) << "\n";

    file.close();
    std::cout << "[Settings] Settings saved to config/settings.ini" << std::endl;
}

void SettingsDialog::LoadSettings() {
    std::ifstream file("config/settings.ini");
    if (!file.is_open()) {
        std::cout << "[Settings] No settings file found, using defaults" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '[' || line[0] == ';') continue;

        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        // Parse settings
        if (key == "master_volume") audio_tab.master_volume = std::stof(value);
        else if (key == "music_volume") audio_tab.music_volume = std::stof(value);
        else if (key == "screen_width") graphics_tab.screen_width = std::stoi(value);
        else if (key == "num_laps") gameplay_tab.num_laps = std::stoi(value);
    }

    file.close();
    std::cout << "[Settings] Settings loaded from config/settings.ini" << std::endl;
}

void SettingsDialog::ApplySettings() {
    // Apply audio settings
    if (audio_system) {
        audio_system->SetMasterVolume(audio_tab.master_volume);
        audio_system->SetMusicVolume(audio_tab.music_volume);
        audio_system->SetEngineVolume(audio_tab.engine_volume);
        audio_system->SetSFXVolume(audio_tab.sfx_volume);
    }

    // Apply graphics settings
    if (render_system) {
        render_system->SetVSync(graphics_tab.vsync_enabled);
        render_system->SetAntiAliasing(graphics_tab.anti_aliasing);
    }
}

void SettingsDialog::OnApply() {
    ApplySettings();
    SaveSettings();
    std::cout << "[Settings] Settings applied and saved" << std::endl;
}

void SettingsDialog::OnReset() {
    ResetToDefaults();
    ApplySettings();
    SaveSettings();
    std::cout << "[Settings] Settings reset to defaults" << std::endl;
}

void SettingsDialog::OnClose() {
    SaveSettings();
    Close();
}

void SettingsDialog::ResetToDefaults() {
    // Audio defaults
    audio_tab.master_volume = 0.8f;
    audio_tab.music_volume = 0.6f;
    audio_tab.engine_volume = 0.7f;
    audio_tab.sfx_volume = 0.8f;

    // Graphics defaults
    graphics_tab.screen_width = 1920;
    graphics_tab.screen_height = 1080;
    graphics_tab.vsync_enabled = true;
    graphics_tab.anti_aliasing = true;

    // Gameplay defaults
    gameplay_tab.difficulty = GameplayTab::DifficultyMode::Normal;
    gameplay_tab.num_laps = 3;
    gameplay_tab.num_racers = 4;
    gameplay_tab.manual_transmission = false;
}

// ========== HELPER FUNCTIONS ==========

bool SettingsDialog::IsMouseInRect(double mx, double my, float x, float y, float w, float h) const {
    return mx >= x && mx <= x + w && my >= y && my <= y + h;
}

void SettingsDialog::PlayTestSound(const std::string& sound_name) {
    if (audio_system) {
        audio_system->PlaySFX(sound_name);
    }
}

void SettingsDialog::PlayEngineSound(float speed_kmh) {
    if (audio_system) {
        audio_system->PlayEngineSound(speed_kmh);
    }
}
