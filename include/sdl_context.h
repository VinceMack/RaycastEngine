#pragma once

#include <SDL2/SDL.h>
#include <cstdint>
#include <vector>

class SDLContext
{
public:
    SDLContext(int width, int height);
    ~SDLContext();

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    std::vector<uint32_t> pixelBuffer;
    bool running = false;
};
