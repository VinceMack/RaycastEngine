#include <SDL2/SDL.h>
#include <cstdint>
#include <iostream>
#include <vector>

constexpr float screen_width = 640;
constexpr float screen_height = 480;
constexpr int map_size = 24;

struct SDL
{
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    uint32_t pixelBuffer[480][640] = {};
    bool running = false;

    SDL(int width, int height)
    {
        // Initalize SDL
        SDL_Init(SDL_INIT_VIDEO);

        // Initalize SDL Window
        window = SDL_CreateWindow(
            "raycaster",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            width,
            height,
            SDL_WINDOW_SHOWN
        );

        // Intalize SDL Rednerer
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        // Initalize SDL Texture
        texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            width,
            height
        );

        // Fill the pixelBuffer. (blue in this case)
        for (int y = 0; y < screen_height; ++y)
        {
            for (int x = 0; x < screen_width; ++x)
                pixelBuffer[y][x] = 0xFF0000FF; // ARGB: opaque blue
        }

        running = true;
    }

    ~SDL()
    {
        if (texture)
            SDL_DestroyTexture(texture);

        if (renderer)
            SDL_DestroyRenderer(renderer);

        if (window)
            SDL_DestroyWindow(window);

        SDL_Quit();
    }
};

struct Vector2 {
    double x, y;

    Vector2 operator*(double scalar) const { return {x * scalar, y * scalar}; }
    Vector2 operator+(const Vector2& other) const { return {x + other.x, y + other.y}; }
    Vector2 operator-(const Vector2& other) const { return {x - other.x, y - other.y}; }
    void operator+=(const Vector2& other) { x += other.x; y += other.y; }
    void operator-=(const Vector2& other) { x -= other.x; y -= other.y; }
};

struct Player
{
    Vector2 pos;
    Vector2 dir;
    Vector2 plane;

    Player(Vector2 pos, Vector2 dir, Vector2 plane) : pos(pos), dir(dir), plane(plane)
    {

    }

    Player()
    {
        pos = {0.0, 0.0};
        dir = {0.0, 0.0};
        plane = {0.0, 0.0};
    }
};

struct Map
{
    int grid[map_size][map_size] =
    {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
    {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
    {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    };

    Map()
    {

    }
};

int main(int argc, char* argv[])
{
    // Set up SDL
    SDL sdl(screen_width, screen_height);
    // Set up player
    Player player;
    // Set up map
    Map map_grid;

    SDL_Event event;
    while (sdl.running)
    {
        // Handle SDL quit event (if the window closes, stop running)
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                sdl.running = false;
        }

        // Upload pixels
        SDL_UpdateTexture(sdl.texture, nullptr, sdl.pixelBuffer, 640 * sizeof(uint32_t));
        // Clear frame
        SDL_RenderClear(sdl.renderer);
        // Draw texture
        SDL_RenderCopy(sdl.renderer, sdl.texture, nullptr, nullptr);
        // Present frame
        SDL_RenderPresent(sdl.renderer);
    }

    return 0;
}
