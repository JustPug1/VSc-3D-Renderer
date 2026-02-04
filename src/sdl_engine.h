#pragma once

#include <SDL3/SDL.h>
#include <iostream>

class SDL_Engine {
private:
    int width; // Width of screen
    int height; // Height of screen
    const char* window_name; // Name of sdl window / screen (pxl)

    // All the necessary flags (enums) for each task
    SDL_InitFlags init_flags;
    SDL_WindowFlags window_flags;

public:
    // We create (window and renderer) in public - for the user to use them outside of the class
    // All of the Render Functions depend on the renderer (it is passed as a pointer)
    SDL_Window* window;
    SDL_Renderer* renderer;

    // Description in the .cpp file
    SDL_Engine(const char* window_name, int w, int h, SDL_InitFlags init_flags, SDL_WindowFlags window_flags);
    
    // Description in the .cpp file
    SDL_Gamepad* Connect_First_Controller();

};