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
    // Check Keyboard Bindings
    const auto it = bindings.find(action);
    if (it != bindings.end())
    {
        const uint8_t* state = SDL_GetKeyboardState(NULL);
        for (SDL_Scancode key : it->second)
        {
            if (state[key]) return true;
        }
    }

    // Special Case: Handle Mouse Firing
    if (action == "Fire")
    {
        return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT);
    }

    return false;
}

void Input::handlePlayerInput(double deltaTime, Player& player, const Map& map_grid)
{
    double frameMoveSpeed = player.moveSpeed * deltaTime;
    double frameRotSpeed = player.rotSpeed * deltaTime;
    double buffer = 0.2;

    // 1. Capture Mouse Motion AND Button States
    int mouseX;
    // We capture the return value (button bitmask)
    uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, nullptr);

    // 2. Handle Rotation (Mouse)
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

    // 3. Handle Rotation (Keyboard)
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

    // 4. Handle Movement
    double moveFwd = 0.0;
    double moveStrafe = 0.0;

    if (isActionPressed("Forward"))     moveFwd += 1.0;
    if (isActionPressed("Backward"))    moveFwd -= 1.0;
    if (isActionPressed("StrafeRight")) moveStrafe += 1.0;
    if (isActionPressed("StrafeLeft"))  moveStrafe -= 1.0;

    // Normalize diagonal movement speed
    double inputLen = sqrt(moveFwd * moveFwd + moveStrafe * moveStrafe);
    if (inputLen > 1.0)
    {
        moveFwd /= inputLen;
        moveStrafe /= inputLen;
    }

    double velocityX = (player.direction.x * moveFwd + player.plane.x * moveStrafe) * frameMoveSpeed;
    double velocityY = (player.direction.y * moveFwd + player.plane.y * moveStrafe) * frameMoveSpeed;

    // Independent axis collision (sliding)
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