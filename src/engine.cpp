#include "engine.h"

#include <string>

Engine::Engine() : sdl(screen_width, screen_height), scene(), renderer(sdl), input()
{
}

void Engine::run()
{
    uint64_t lastCounter = SDL_GetPerformanceCounter();
    double freq = (double)SDL_GetPerformanceFrequency();

    SDL_Event event;
    while (sdl.running)
    {
        uint64_t currentCounter = SDL_GetPerformanceCounter();
        double deltaTime = (double)(currentCounter - lastCounter) / freq;
        lastCounter = currentCounter;

        updateWindowTitle(deltaTime);
        processEvents(event);
        update(deltaTime);
        renderer.render(scene);
    }
}

void Engine::processEvents(SDL_Event& event)
{
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            sdl.running = false;
        }
    }
}

void Engine::update(double deltaTime)
{
    input.handlePlayerInput(deltaTime, scene.player, scene.map_grid);
}

void Engine::updateWindowTitle(double deltaTime)
{
    double msPerFrame = deltaTime * 1000.0;
    SDL_SetWindowTitle(sdl.window, ("Raycaster - Frame Time: " + std::to_string(msPerFrame) + "ms").c_str());
}
