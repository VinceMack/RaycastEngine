#pragma once

#include "constants.h"
#include "asset_manager.h"
#include "input.h"
#include "renderer.h"
#include "scene.h"
#include "sdl_context.h"

class Engine
{
public:
    Engine();
    void run();

private:
    SDLContext sdl;
    AssetManager assetManager;
    Scene scene;
    Renderer renderer;
    Input input;
    bool wasFirePressedLastFrame = false;

    void fireWeapon();
    void processEvents(SDL_Event& event);
    void update(double deltaTime);
    void updateWindowTitle(double deltaTime);
    void updateEntities(double deltaTime);
    void updateEnemyAI(Entity& e, double deltaTime, int& pathsCalculated);
    void updateSmartAI(Entity& e, double deltaTime, int& pathsCalculated);
    std::vector<Vector2> calculateAStarPath(Vector2 start, Vector2 target);
};