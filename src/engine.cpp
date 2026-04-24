#include "engine.h"

#include <string>
#include <cmath>

Engine::Engine() : sdl(screen_width, screen_height), assetManager(), scene(assetManager), renderer(sdl, assetManager), input()
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

#include <algorithm> // Required for std::remove_if

void Engine::updateEntities(double deltaTime)
{
    // 1. Process Logic and Mark for Deletion
    for (auto& e : scene.entities)
    {
        e.totalTime += deltaTime;

        // Pickup Logic
        if (e.type == EntityType::ITEM) 
        {
            Vector2 distVec = e.position - scene.player.position;
            double distSq = distVec.x * distVec.x + distVec.y * distVec.y;

            if (distSq < 0.25) // 0.5 units distance pickup radius
            { 
                if (e.weaponInside != WeaponID::NONE)
                {
                    scene.player.currentWeapon = assetManager.getWeaponDefinition(e.weaponInside);
                }
                e.dist = -1.0; // Marker: mark for deletion
                continue;      // Skip behavioral logic for this frame
            }
        }

        // Behavior Logic
        switch (e.type)
        {
            case EntityType::ITEM:
                e.vOffset = 0.8 + std::sin(e.totalTime * 3.0) * 0.1;
                break;

            case EntityType::ENEMY:
                updateEnemyAI(e, deltaTime); 
                break;

            default:
                break;
        }
    }

    // 2. Physical Deletion (Erase-Remove Idiom)
    // This moves all "marked" entities to the end and shrinks the vector.
    scene.entities.erase(
        std::remove_if(scene.entities.begin(), scene.entities.end(),
            [](const Entity& e) { return e.dist < 0.0; }), 
        scene.entities.end()
    );
}