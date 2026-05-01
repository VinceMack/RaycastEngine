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

enum class EntityType
{
    STATIC,     // Doesn't move (Lamps, decorations)
    ITEM,       // Bounces and rotates (Health packs, ammo)
    ENEMY       // Pathfinds and chases player
};

enum class WeaponID
{
    NONE,
    PISTOL,
    SMG
};

struct WeaponDefinition
{
    WeaponID id;
    std::string name;      // For UI display
    int worldTextureIndex; // For the ITEM on the floor
    int handTextureIndex;  // For the gun in the hand
};

enum class AIBehavior {
    DIRECT,  // Current logic: ignore walls, move straight to player
    SMART,   // New logic: A* pathfinding, respect walls
    NONE
};

struct Entity
{
    Vector2 position;
    Vector2 velocity;
    int textureIndex;
    WeaponID weaponInside = WeaponID::NONE;
    int numFrames;
    float frameTimer;
    float frameDuration;
    int currentFrame;
    double scale;
    double vOffset;
    double dist;
    EntityType type;
    double totalTime;                           // accumulated time for math effects
    AIBehavior behavior = AIBehavior::SMART;
    std::vector<Vector2> currentPath;           // Coordinates of grid cells to follow
    float pathTimer = 0.0f;                     // Recalculate path every X seconds

    Entity(Vector2 pos, int texIdx, EntityType t = EntityType::STATIC, 
           AIBehavior b = AIBehavior::DIRECT, double s = 1.0, double vOff = 1.0)
        : position(pos), velocity({0,0}), textureIndex(texIdx), type(t), behavior(b),
          scale(s), vOffset(vOff), numFrames(1), currentFrame(0), frameTimer(0), dist(0) 
    {}
};

struct Node
{
    int x, y;
    double gCost; // Distance from start
    double hCost; // Distance to end (heuristic)
    Node* parent = nullptr;

    double fCost() const { return gCost + hCost; }
    
    bool operator>(const Node& other) const { return (gCost + hCost) > (other.gCost + other.hCost); }
};

struct Player
{
    Vector2 position;
    Vector2 direction;
    Vector2 plane;
    double moveSpeed;
    double rotSpeed;
    double mouseSensitivity;
    const WeaponDefinition* currentWeapon = nullptr;

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
                // Engine layout (at(x,y)) is [x * height + y]
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
