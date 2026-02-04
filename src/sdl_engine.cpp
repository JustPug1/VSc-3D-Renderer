#include "sdl_engine.h"
#include <iostream>

SDL_Engine::SDL_Engine(const char* window_name, int w, int h, SDL_InitFlags init_flags, SDL_WindowFlags window_flags) {
    // Set and declare all given variables 
    this->window_name = window_name;
    this->width = w;
    this->height = h;
    this->init_flags = init_flags;
    this->window_flags = window_flags;

    // The usual process of creating an SDL window and renderer
    // Initialize SDL - ...Later add exception thrown...
    if (!SDL_Init(this->init_flags)) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
    }
    std::cout << "SDL Initialized Successfully!" << std::endl;

    // Create a Window by the given parameters - ...Later add exception thrown...
    this->window = SDL_CreateWindow(this->window_name, this->width, this->height, this->window_flags);
    if(this->window == nullptr) {
        std::cerr << "Failed to create a window: " << SDL_GetError() << std::endl;
        SDL_Quit();
    }

    // Create a Renderer (with the created window) - ...Later add exception thrown...
    this->renderer = SDL_CreateRenderer(this->window, nullptr);
    if (this->renderer == nullptr) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
}

SDL_Gamepad* SDL_Engine::Connect_First_Controller() {
    // Searches for the first available / found controller,
    // connects to it and if disconnected - 
    // (NOTE) it doesnt reconnect 
    // only onetime function - can be called
    // on order to reconnect if needed

    // ..Later add exception thrown...
    
    int count = 0;
    SDL_JoystickID* gamepads = SDL_GetGamepads(&count);
    
    SDL_Gamepad* handle = nullptr;

    if (count > 0) {
        handle = SDL_OpenGamepad(gamepads[0]);
        if (handle) {
            SDL_Log("Successfully connected to: %s", SDL_GetGamepadName(handle));
        } else {
            SDL_Log("Failed to open gamepad: %s", SDL_GetError());
        }
    } else {
        SDL_Log("No gamepads detected.");
    }
    SDL_free(gamepads);
    
    return handle;
}
