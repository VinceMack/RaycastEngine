#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <SDL2/SDL.h>

#include "map.h"
#include "types.h"

class Input
{
public:
    Input();

    void bindAction(const std::string& action, SDL_Scancode key);
    bool isActionPressed(const std::string& action) const;
    void handlePlayerInput(double deltaTime, Player& player, const Map& map_grid);

private:
    std::unordered_map<std::string, std::vector<SDL_Scancode>> bindings;
};
