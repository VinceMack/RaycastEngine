#include "input.h"

#include <cmath>

Input::Input()
{
    bindAction("Forward", SDL_SCANCODE_W);
    bindAction("Forward", SDL_SCANCODE_UP);
    bindAction("Backward", SDL_SCANCODE_S);
    bindAction("Backward", SDL_SCANCODE_DOWN);
    bindAction("StrafeLeft", SDL_SCANCODE_A);
    bindAction("StrafeRight", SDL_SCANCODE_D);
    bindAction("TurnLeft", SDL_SCANCODE_LEFT);
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
    double buffer = 0.2;

    int mouseX;
    SDL_GetRelativeMouseState(&mouseX, nullptr);
    if(mouseX != 0)
    {
        double rotAmount = mouseX * player.mouseSensitivity * deltaTime;
        double oldDirX = player.direction.x;
        player.direction.x = player.direction.x * cos(rotAmount) - player.direction.y * sin(rotAmount);
        player.direction.y = oldDirX * sin(rotAmount) + player.direction.y * cos(rotAmount);

        oldDirX = player.plane.x;
        player.plane.x = player.plane.x * cos(rotAmount) - player.plane.y * sin(rotAmount);
        player.plane.y = oldDirX * sin(rotAmount) + player.plane.y * cos(rotAmount);
    }
    if (isActionPressed("TurnRight") || isActionPressed("TurnLeft"))
    {
        double rotDir = isActionPressed("TurnRight") ? 1.0 : -1.0;
        double rotAmount = rotDir * frameRotSpeed;
        double oldDirX = player.direction.x;
        player.direction.x = player.direction.x * cos(rotAmount) - player.direction.y * sin(rotAmount);
        player.direction.y = oldDirX * sin(rotAmount) + player.direction.y * cos(rotAmount);

        oldDirX = player.plane.x;
        player.plane.x = player.plane.x * cos(rotAmount) - player.plane.y * sin(rotAmount);
        player.plane.y = oldDirX * sin(rotAmount) + player.plane.y * cos(rotAmount);
    }

    double moveFwd = 0.0;
    double moveStrafe = 0.0;

    if (isActionPressed("Forward"))     moveFwd += 1.0;
    if (isActionPressed("Backward"))    moveFwd -= 1.0;
    if (isActionPressed("StrafeRight")) moveStrafe += 1.0;
    if (isActionPressed("StrafeLeft"))  moveStrafe -= 1.0;

    double inputLen = sqrt(moveFwd * moveFwd + moveStrafe * moveStrafe);
    if (inputLen > 1.0)
    {
        moveFwd /= inputLen;
        moveStrafe /= inputLen;
    }

    double velocityX = (player.direction.x * moveFwd + player.plane.x * moveStrafe) * frameMoveSpeed;
    double velocityY = (player.direction.y * moveFwd + player.plane.y * moveStrafe) * frameMoveSpeed;

    if (velocityX != 0)
    {
        double checkX = player.position.x + velocityX + (velocityX > 0 ? buffer : -buffer);
        if (map_grid.at((int)checkX, (int)player.position.y) == 0)
            player.position.x += velocityX;
    }

    if (velocityY != 0)
    {
        double checkY = player.position.y + velocityY + (velocityY > 0 ? buffer : -buffer);
        if (map_grid.at((int)player.position.x, (int)checkY) == 0)
            player.position.y += velocityY;
    }
}