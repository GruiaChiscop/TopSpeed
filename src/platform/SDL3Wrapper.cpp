#include "SDL3Wrapper.h"
#include <iostream>
#include <cstring>

namespace TopSpeed {

SDL3Wrapper::SDL3Wrapper()
    : window_(nullptr)
    , renderer_(nullptr)
    , window_width_(800)
    , window_height_(600)
    , should_quit_(false) {
}

SDL3Wrapper::~SDL3Wrapper() {
    Shutdown();
}

bool SDL3Wrapper::Initialize() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS)) {
        std::cerr << "Failed to initialize SDL3: " << SDL_GetError() << std::endl;
        return false;
    }

    // Initialize keyboard state tracking
    keyboard_state_.resize(SDL_NUM_SCANCODES, false);

    return true;
}

bool SDL3Wrapper::CreateWindow(int width, int height, const std::string& title) {
    window_width_ = width;
    window_height_ = height;

    window_ = SDL_CreateWindow(
        title.c_str(),
        width,
        height,
        SDL_WINDOW_RESIZABLE
    );

    if (!window_) {
        std::cerr << "Failed to create SDL window: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, nullptr, SDL_RENDERER_ACCELERATED | SDL_RENDERER_VSYNC);
    if (!renderer_) {
        std::cerr << "Failed to create SDL renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        return false;
    }

    // Set default drawing color to black
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);

    return true;
}

bool SDL3Wrapper::ProcessEvents() {
    should_quit_ = false;

    // Update keyboard state
    std::fill(keyboard_state_.begin(), keyboard_state_.end(), false);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                should_quit_ = true;
                break;

            case SDL_EVENT_KEY_DOWN:
                if (event.key.scancode < keyboard_state_.size()) {
                    keyboard_state_[event.key.scancode] = true;
                }
                break;

            case SDL_EVENT_KEY_UP:
                if (event.key.scancode < keyboard_state_.size()) {
                    keyboard_state_[event.key.scancode] = false;
                }
                break;

            case SDL_EVENT_WINDOW_RESIZED:
                window_width_ = event.window.data1;
                window_height_ = event.window.data2;
                break;

            case SDL_EVENT_WINDOW_FOCUS_LOST:
                // Could pause game here
                break;

            default:
                break;
        }
    }

    return !should_quit_;
}

const bool* SDL3Wrapper::GetKeyboardState() const {
    return keyboard_state_.data();
}

bool SDL3Wrapper::IsKeyPressed(SDL_Scancode scancode) const {
    if (scancode >= 0 && scancode < keyboard_state_.size()) {
        return keyboard_state_[scancode];
    }
    return false;
}

void SDL3Wrapper::ClearScreen(uint8_t r, uint8_t g, uint8_t b) {
    if (renderer_) {
        SDL_SetRenderDrawColor(renderer_, r, g, b, 255);
        SDL_RenderClear(renderer_);
    }
}

void SDL3Wrapper::Present() {
    if (renderer_) {
        SDL_RenderPresent(renderer_);
    }
}

std::pair<int, int> SDL3Wrapper::GetWindowSize() const {
    return {window_width_, window_height_};
}

void SDL3Wrapper::SetWindowSize(int width, int height) {
    if (window_) {
        SDL_SetWindowSize(window_, width, height);
        window_width_ = width;
        window_height_ = height;
    }
}

void SDL3Wrapper::Shutdown() {
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }

    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }

    SDL_Quit();
}

} // namespace TopSpeed
