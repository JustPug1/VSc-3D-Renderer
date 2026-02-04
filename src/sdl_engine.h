#pragma once

#include <SDL3/SDL.h>
#include <iostream>

class SDL_Engine {
private:
    int width;
    int height;
    const char* window_name;
    SDL_InitFlags init_flags;
    SDL_WindowFlags window_flags;

public:
    SDL_Window* window;
    SDL_Renderer* renderer;

    SDL_Engine(const char* window_name, int w, int h, SDL_InitFlags init_flags, SDL_WindowFlags window_flags);

    SDL_Gamepad* Connect_First_Controller();

};