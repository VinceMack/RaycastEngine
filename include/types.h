#pragma once

#include <cstdint>
#include <vector>
#include <SDL2/SDL_image.h>
#include <string>
#include <stdexcept>

struct Vector2
{
    double x, y;

    Vector2 operator*(double scalar) const { return {x * scalar, y * scalar}; }
    Vector2 operator+(const Vector2& other) const { return {x + other.x, y + other.y}; }
    Vector2 operator-(const Vector2& other) const { return {x - other.x, y - other.y}; }
    void operator+=(const Vector2& other) { x += other.x; y += other.y; }
    void operator-=(const Vector2& other) { x -= other.x; y -= other.y; }
};

struct Sprite
{
    Vector2 position;
    int textureIndex;
    double dist;
    double scale;
};

struct Player
{
    Vector2 position;
    Vector2 direction;
    Vector2 plane;
    double moveSpeed;
    double rotSpeed;
    double mouseSensitivity;

    Player(Vector2 pos, Vector2 dir, Vector2 planeVec, double rotationSpeed, double movementSpeed, double mouseSens)
        : position(pos), direction(dir), plane(planeVec), moveSpeed(movementSpeed), rotSpeed(rotationSpeed), mouseSensitivity(mouseSens)
    {
    }

    Player()
        : position({22.0, 12.0}), direction({0.0, -1.0}), plane({0.66, 0.0}), moveSpeed(5), rotSpeed(3), mouseSensitivity(1)
    {
    }
};

struct Texture
{
    int width = 0;
    int height = 0;
    std::vector<uint32_t> pixels;

    Texture() = default;

    Texture(int w, int h)
    {
        resize(w, h);
    }

    void resize(int w, int h)
    {
        width = w;
        height = h;
        pixels.resize(width * height);
    }

    uint32_t& at(int x, int y)
    {
        return pixels[x * height + y];
    }

    const uint32_t& at(int x, int y) const
    {
        return pixels[x * height + y];
    }

    void loadFromFile(const std::string& path)
    {
        SDL_Surface* tempSurface = IMG_Load(path.c_str());
        if (!tempSurface)
        {
            throw std::runtime_error("Failed to load texture: " + path);
        }

        // 1. Force conversion to ARGB8888 to fix the Red/Blue color swap
        SDL_Surface* surface = SDL_ConvertSurfaceFormat(tempSurface, SDL_PIXELFORMAT_ARGB8888, 0);
        SDL_FreeSurface(tempSurface); // We don't need the original anymore

        width = surface->w;
        height = surface->h;
        pixels.resize(width * height);

        uint32_t* srcPixels = (uint32_t*)surface->pixels;

        // 2. Convert Row-Major (file) to Column-Major (engine) to fix the 90° rotation
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                // The file layout is [y * width + x]
                // Your engine layout (at(x,y)) is [x * height + y]
                this->at(x, y) = srcPixels[y * width + x];
            }
        }

        SDL_FreeSurface(surface);
    }
};

struct RayHit
{
    double distance;
    int wallType;
    int side;
    int mapX;
    int mapY;
    Vector2 rayDir;
};
