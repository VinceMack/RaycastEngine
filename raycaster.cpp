#include <SDL2/SDL.h>
#include <cstdint>
#include <iostream>
#include <vector>
#include <cmath>

constexpr double screen_width = 640;
constexpr double screen_height = 480;
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

        running = true;
    }

    void fillBuffer(uint32_t color)
    {
        // Fill the pixelBuffer.
        for (int y = 0; y < screen_height; ++y)
        {
            for (int x = 0; x < screen_width; ++x)
                pixelBuffer[y][x] = color;
        }
    }

    void fillBuffer(uint32_t bottomColor, uint32_t topColor)
    {
        // Fill the top half of pixelBuffer.
        for (int y = 0; y < screen_height / 2; ++y)
        {
            for (int x = 0; x < screen_width; ++x)
            {
                pixelBuffer[y][x] = bottomColor;
            }
        }

        // Fill the bottom half of pixelBuffer.
        for (int y = screen_height / 2; y < screen_height; ++y)
        {
            for (int x = 0; x < screen_width; ++x)
            {
                pixelBuffer[y][x] = topColor;
            }
        }
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

struct Vector2
{
    double x, y;

    Vector2 operator*(double scalar) const { return {x * scalar, y * scalar}; }
    Vector2 operator+(const Vector2& other) const { return {x + other.x, y + other.y}; }
    Vector2 operator-(const Vector2& other) const { return {x - other.x, y - other.y}; }
    void operator+=(const Vector2& other) { x += other.x; y += other.y; }
    void operator-=(const Vector2& other) { x -= other.x; y -= other.y; }
};

struct Point
{
    double x, y;

    Vector2 operator*(double scalar) const { return {x * scalar, y * scalar}; }
    Vector2 operator+(const Vector2& other) const { return {x + other.x, y + other.y}; }
    Vector2 operator-(const Vector2& other) const { return {x - other.x, y - other.y}; }
    void operator+=(const Vector2& other) { x += other.x; y += other.y; }
    void operator-=(const Vector2& other) { x -= other.x; y -= other.y; }
};

struct Player
{
    Point position;
    Vector2 direction;
    Vector2 plane;

    Player(Point pos, Vector2 dir, Vector2 plane) : plane(plane)
    {
        position = pos;
        direction = dir;
    }

    Player()
    {
        position = {22.0, 12.0};
        direction = {0.0, -1.0};
        plane = {0.66, 0.0};
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

    // Handle frame.
    SDL_Event event;
    while (sdl.running)
    {
        // Handle SDL quit event (if the window closes, stop running)
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                sdl.running = false;
        }

        // Reset the pixel buffer:
        // Fill the top half of the pixel buffer (celining)
        // Fill the bottom half of the pixel buffer (floor)
        sdl.fillBuffer(0xFF0000FF, 0xFFC0C0C0);
        
        // Iterate over every vertical column of the screen:
        double cameraX;
        Vector2 rayDir;
        for(int x=0; x < screen_width; x++)
        {
            // Calculate cameraX (the x-coordinate in camera space, from -1 to 1).
            cameraX = 2.0 * x / screen_width - 1;

            // Calculate the ray direction vector for the current column.
            rayDir = player.direction + (player.plane * cameraX);

            // Determine the step direction (+1 or -1) for grid traversal.
            int step_x;
            if (rayDir.x > 0)       step_x = 1;
            else if (rayDir.x < 0)  step_x = -1;
            else                    step_x = 0; // Ray is perfectly vertical; no X-step needed.

            int step_y;
            if (rayDir.y > 0)       step_y = 1;
            else if (rayDir.y < 0)  step_y = -1;
            else                    step_y = 0; // Ray is perfectly horizontal; no Y-step needed.

            // Identify the starting/current map square (integer coordinates).
            int ray_curr_grid_x = (int)player.position.x;
            int ray_curr_grid_y = (int)player.position.y;

            // Calculate the total length the ray travels to cross one full grid square in X or Y.
            double ray_step_cost_x = fabs(1.0 / rayDir.x);
            double ray_step_cost_y = fabs(1.0 / rayDir.y);

            // Calculate the initial distance from the player to the first grid boundaries.
            double dist_to_next_grid_line_x;
            if(rayDir.x > 0)        dist_to_next_grid_line_x = (ray_curr_grid_x + 1.0 - player.position.x) * ray_step_cost_x;
            else if(rayDir.x < 0)   dist_to_next_grid_line_x = (player.position.x - ray_curr_grid_x) * ray_step_cost_x;
            else                    dist_to_next_grid_line_x = INFINITY;
            
            double dist_to_next_grid_line_y;
            if(rayDir.y > 0)        dist_to_next_grid_line_y = (ray_curr_grid_y + 1.0 - player.position.y) * ray_step_cost_y;
            else if(rayDir.y < 0)   dist_to_next_grid_line_y = (player.position.y - ray_curr_grid_y) * ray_step_cost_y;
            else                    dist_to_next_grid_line_y = INFINITY;

            // Perform the DDA Loop until a wall is hit.
            bool hit = false;
            while(!hit)
            {
                // TODO: IMPLEMENT DDA LOOP
            }

            // Calculate the final_wall_dist (perpendicular distance).
            // Calculate line_height, draw_start, and draw_end.
            // Draw the vertical line into your pixelBuffer.
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

// cameraX & rayDir
// cameraX is a normalized coordinate that maps every vertical stripe of your screen to a value between -1 and 1.
// It tells the engine where the current ray is relative to the center of the player's view:
// We use this value to "stretch" the Camera Plane vector. By multiplying the plane vector by cameraX, we find the offset from the center.
// Adding this offset to the Direction vector gives us the unique direction for that specific ray: rayDir = dir + (plane * cameraX)
// This ensures that the rays fan out correctly to cover your entire Field of View (FOV).