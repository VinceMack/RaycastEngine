#include "engine.h"

#include <string>
#include <cmath>

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
    updateEntities(deltaTime);
}

void Engine::updateWindowTitle(double deltaTime)
{
    double msPerFrame = deltaTime * 1000.0;
    SDL_SetWindowTitle(sdl.window, ("Raycaster - Frame Time: " + std::to_string(msPerFrame) + "ms").c_str());
}

void Engine::updateEnemyAI(Entity& e, double deltaTime) {
    Vector2 dirToPlayer = scene.player.position - e.position;
    double distance = std::sqrt(dirToPlayer.x * dirToPlayer.x + dirToPlayer.y * dirToPlayer.y);

    // Only chase if player is within 10 units
    if (distance < 10.0 && distance > 0.5) {
        // Normalize direction
        dirToPlayer.x /= distance;
        dirToPlayer.y /= distance;

        double moveSpeed = 2.0;
        e.position.x += dirToPlayer.x * moveSpeed * deltaTime;
        e.position.y += dirToPlayer.y * moveSpeed * deltaTime;
    }
}

void Engine::updateEntities(double deltaTime)
{
    for (auto& e : scene.entities)
    {
        e.totalTime += deltaTime;

        switch (e.type)
        {
            case EntityType::ITEM:
                // Visual "Bobbing" effect using a Sine wave
                // Modulate vOffset around 1.0 (the floor)
                e.vOffset = 0.8 + std::sin(e.totalTime * 3.0) * 0.1;
                break;

            case EntityType::ENEMY:
                updateEnemyAI(e, deltaTime); // simple testing pathfinding
                break;

            case EntityType::STATIC:
            default:
                break;
        }
    }
}