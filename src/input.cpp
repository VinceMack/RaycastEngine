#include "input.h"

#include <cmath>

Input::Input()
{
    bindAction("Forward", SDL_SCANCODE_W);
    bindAction("Forward", SDL_SCANCODE_UP);
    bindAction("Backward", SDL_SCANCODE_S);
    bindAction("Backward", SDL_SCANCODE_DOWN);
    bindAction("TurnLeft", SDL_SCANCODE_A);
    bindAction("TurnLeft", SDL_SCANCODE_LEFT);
    bindAction("TurnRight", SDL_SCANCODE_D);
    bindAction("TurnRight", SDL_SCANCODE_RIGHT);
}

void Input::bindAction(const std::string& action, SDL_Scancode key)
{
    bindings[action].push_back(key);
}

bool Input::isActionPressed(const std::string& action) const
{
    const auto it = bindings.find(action);
    if (it == bindings.end()) return false;

    const uint8_t* state = SDL_GetKeyboardState(NULL);
    for (SDL_Scancode key : it->second)
    {
        if (state[key]) return true;
    }

    return false;
}

void Input::handlePlayerInput(double deltaTime, Player& player, const Map& map_grid)
{
    double frameMoveSpeed = player.moveSpeed * deltaTime;
    double frameRotSpeed = player.rotSpeed * deltaTime;

    if (isActionPressed("Forward"))
    {
        double newX = player.position.x + player.direction.x * frameMoveSpeed;
        if (map_grid.at((int)newX, (int)player.position.y) == 0)
        {
            player.position.x = newX;
        }

        double newY = player.position.y + player.direction.y * frameMoveSpeed;
        if (map_grid.at((int)player.position.x, (int)newY) == 0)
        {
            player.position.y = newY;
        }
    }
    if (isActionPressed("Backward"))
    {
        double newX = player.position.x - player.direction.x * frameMoveSpeed;
        if (map_grid.at((int)newX, (int)player.position.y) == 0)
        {
            player.position.x = newX;
        }

        double newY = player.position.y - player.direction.y * frameMoveSpeed;
        if (map_grid.at((int)player.position.x, (int)newY) == 0)
        {
            player.position.y = newY;
        }
    }
    if (isActionPressed("TurnRight"))
    {
        double oldDirX = player.direction.x;
        player.direction.x = player.direction.x * cos(frameRotSpeed) - player.direction.y * sin(frameRotSpeed);
        player.direction.y = oldDirX * sin(frameRotSpeed) + player.direction.y * cos(frameRotSpeed);

        oldDirX = player.plane.x;
        player.plane.x = player.plane.x * cos(frameRotSpeed) - player.plane.y * sin(frameRotSpeed);
        player.plane.y = oldDirX * sin(frameRotSpeed) + player.plane.y * cos(frameRotSpeed);
    }
    if (isActionPressed("TurnLeft"))
    {
        double oldDirX = player.direction.x;
        player.direction.x = player.direction.x * cos(-frameRotSpeed) - player.direction.y * sin(-frameRotSpeed);
        player.direction.y = oldDirX * sin(-frameRotSpeed) + player.direction.y * cos(-frameRotSpeed);

        oldDirX = player.plane.x;
        player.plane.x = player.plane.x * cos(-frameRotSpeed) - player.plane.y * sin(-frameRotSpeed);
        player.plane.y = oldDirX * sin(-frameRotSpeed) + player.plane.y * cos(-frameRotSpeed);
    }
}
