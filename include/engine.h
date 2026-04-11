#pragma once

#include "constants.h"
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
    Scene scene;
    Renderer renderer;
    Input input;

    void processEvents(SDL_Event& event);
    void update(double deltaTime);
    void updateWindowTitle(double deltaTime);
};
