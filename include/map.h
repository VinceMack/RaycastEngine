#pragma once

#include <vector>

#include "constants.h"

class Map
{
public:
    Map();
    int at(int x, int y) const;

private:
    std::vector<int> grid;
};
