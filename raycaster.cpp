#include <SDL2/SDL.h>
#include <cstdint>
#include <iostream>
#include <vector>
#include <cmath>

constexpr double screen_width = 640;
constexpr double screen_height = 480;
constexpr int map_size = 24;
constexpr int textureHeight = 64;
constexpr int textureWidth = 64;

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
            for (int x = 0; x < screen_width; ++x)
                pixelBuffer[y][x] = color;
    }

    void fillBuffer(uint32_t topColor, uint32_t bottomColor)
    {
        // Fill the top half of pixelBuffer (Ceiling)
        for (int y = 0; y < screen_height / 2; ++y)
            for (int x = 0; x < screen_width; ++x)
                pixelBuffer[y][x] = topColor;

        // Fill the bottom half of pixelBuffer (Floor)
        for (int y = screen_height / 2; y < screen_height; ++y)
            for (int x = 0; x < screen_width; ++x)
                pixelBuffer[y][x] = bottomColor;
    }

    ~SDL()
    {
        if (texture) SDL_DestroyTexture(texture);
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
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

struct Player
{
    Vector2 position;
    Vector2 direction;
    Vector2 plane;
    double moveSpeed;
    double rotSpeed;

    Player(Vector2 pos, Vector2 dir, Vector2 plane, double rotationSpeed, double movementSpeed) : plane(plane)
    {
        position = pos;
        direction = dir;
        moveSpeed = movementSpeed;
        rotSpeed = rotationSpeed;
    }

    Player()
    {
        position = {22.0, 12.0};
        direction = {0.0, -1.0};
        plane = {0.66, 0.0};
        moveSpeed = 0.005;
        rotSpeed = 0.003;
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

void generateTexture(std::vector<uint32_t> &texture, uint32_t brickColor)
{
    texture.resize(64 * 64);
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            bool edge = (y % 16 == 0) || ((x + (y / 16) * 16) % 32 == 0);
            texture[y * 64 + x] = edge ? 0xFF000000 : brickColor;
        }
    }
}

int main(int argc, char* argv[])
{
    // Set up SDL
    SDL sdl(screen_width, screen_height);
    // Set up player
    Player player;
    // Set up map
    Map map_grid;

    // Initialize all 8 textures in your main loop
    std::vector<uint32_t> texture[8];
    
    uint32_t colors[8] = { 0xFFFF0000, 0xFF00FF00, 0xFF0000FF, 0xFFFFFF00, 
                        0xFFFF00FF, 0xFF00FFFF, 0xFFFFFFFF, 0xFF777777 };

    for (int i = 0; i < 8; i++) {
        generateTexture(texture[i], colors[i]);
    }

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

        // Handle Player Movement
        const uint8_t* state = SDL_GetKeyboardState(NULL);
        if (state[SDL_SCANCODE_W]) 
        {
            // Calcualte the new potential position by adding the direction vector multiplied by the movement speed to the current position.
            // If the potential position is valid (i.e. not in a wall), update the player's position to the new position.
            // Repeat with both the x and y coordinates, checking each separately to allow for sliding along walls.
            double newX = player.position.x + player.direction.x * player.moveSpeed;
            if(map_grid.grid[(int)newX][(int)player.position.y] == 0)
            {
                player.position.x = newX;
            }

            double newY = player.position.y + player.direction.y * player.moveSpeed;
            if(map_grid.grid[(int)player.position.x][(int)newY] == 0)
            {
                player.position.y = newY;
            }
        }
        if (state[SDL_SCANCODE_S])
        {
            // Calculate the new potential position by subtracting the direction vector multiplied by the movement speed from the current position.
            // If the potential position is valid (i.e. not in a wall), update the player's position to the new position.
            // Repeat with both the x and y coordinates, checking each separately to allow for sliding along walls.
            double newX = player.position.x - player.direction.x * player.moveSpeed;
            if(map_grid.grid[(int)newX][(int)player.position.y] == 0)
            {
                player.position.x = newX;
            }

            double newY = player.position.y - player.direction.y * player.moveSpeed;
            if(map_grid.grid[(int)player.position.x][(int)newY] == 0)
            {
                player.position.y = newY;
            }
        }
        if (state[SDL_SCANCODE_D])
        {
            // Rotate the direction vector and camera plane vector to the right by multiplying them with the appropriate rotation matrix.
            // Repeat the same rotation for the camera plane vector.
            double oldDirX = player.direction.x;
            player.direction.x = player.direction.x * cos(player.rotSpeed) - player.direction.y * sin(player.rotSpeed);
            player.direction.y = oldDirX * sin(player.rotSpeed) + player.direction.y * cos(player.rotSpeed);

            oldDirX = player.plane.x;
            player.plane.x = player.plane.x * cos(player.rotSpeed) - player.plane.y * sin(player.rotSpeed);
            player.plane.y = oldDirX * sin(player.rotSpeed) + player.plane.y * cos(player.rotSpeed);
        }
        if (state[SDL_SCANCODE_A])
        {
            // Rotate the direction vector and camera plane vector to the left by multiplying them with the appropriate rotation matrix.
            // Repeat the same rotation for the camera plane vector.
            double oldDirX = player.direction.x;
            player.direction.x = player.direction.x * cos(-player.rotSpeed) - player.direction.y * sin(-player.rotSpeed);
            player.direction.y = oldDirX * sin(-player.rotSpeed) + player.direction.y * cos(-player.rotSpeed);

            oldDirX = player.plane.x;
            player.plane.x = player.plane.x * cos(-player.rotSpeed) - player.plane.y * sin(-player.rotSpeed);
            player.plane.y = oldDirX * sin(-player.rotSpeed) + player.plane.y * cos(-player.rotSpeed);
        }

        // Reset the pixel buffer:
        // Fill the top half of the pixel buffer (ceiling)
        // Fill the bottom half of the pixel buffer (floor)
        sdl.fillBuffer(0xFF91D2FF, 0xFF50404D);
        
        // Iterate over every vertical column of the screen:
        double cameraX;
        Vector2 rayDir;
        for(int x=0; x < screen_width; x++)
        {
            // Calculate cameraX (the x-coordinate in camera space, from -1 to 1).
            cameraX = 2.0 * x / screen_width - 1;

            // Calculate the ray direction vector for the current column.
            rayDir = player.direction + (player.plane * cameraX);

            // Determine the step direction (+1 or -1) for grid traversal, for both x and y.
            int step_dir_x;
            if (rayDir.x > 0)       step_dir_x = 1;
            else if (rayDir.x < 0)  step_dir_x = -1;
            else                    step_dir_x = 0; // Ray is perfectly vertical; no X-step needed.

            int step_dir_y;
            if (rayDir.y > 0)       step_dir_y = 1;
            else if (rayDir.y < 0)  step_dir_y = -1;
            else                    step_dir_y = 0; // Ray is perfectly horizontal; no Y-step needed.

            // Identify the starting/current map square (integer coordinates), for both x and y.
            int ray_curr_grid_x = (int)player.position.x;
            int ray_curr_grid_y = (int)player.position.y;

            // Calculate the total length the ray travels to cross one full grid square, for both x and y.
            double ray_step_cost_x = fabs(1.0 / rayDir.x);
            double ray_step_cost_y = fabs(1.0 / rayDir.y);

            // Calculate the initial distance from the player to the first grid boundariesm, for both x and y.
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
            int side = 0; // 0 for vertical wall, 1 for horizontal wall
            while(!hit)
            {
                // Compare dist_to_next_grid_line_x and dist_to_next_grid_line_y
                // Increment the smaller distance by its corresponding step cost.
                // Update the corresponding current grid coordinate by its step direction.
                // Set side (0 for X, 1 for Y).
                if(dist_to_next_grid_line_x > dist_to_next_grid_line_y)
                {
                    dist_to_next_grid_line_y += ray_step_cost_y;
                    ray_curr_grid_y += step_dir_y;
                    side = 1;
                }
                else
                {
                    dist_to_next_grid_line_x += ray_step_cost_x;
                    ray_curr_grid_x += step_dir_x;
                    side = 0;
                }

                // Check if map_grid.grid[ray_curr_grid_x][ray_curr_grid_y] > 0. If so, hit = true.
                if(map_grid.grid[ray_curr_grid_x][ray_curr_grid_y] > 0) hit = true;
            }

            // Calculate the final_wall_dist (perpendicular distance).
            double final_wall_dist;
            if(side == 0)   final_wall_dist = dist_to_next_grid_line_x - ray_step_cost_x;
            else            final_wall_dist = dist_to_next_grid_line_y - ray_step_cost_y;

            // Calculate line_height, draw_start, and draw_end.
            int line_height;
            line_height = screen_height / final_wall_dist;
            int draw_start;
            draw_start = -(line_height/2)+(screen_height/2);
            int draw_end;
            draw_end = (line_height/2)+(screen_height/2);
            // Clamp these values, if necessary.
            if(draw_start < 0) draw_start = 0;
            if(draw_end >= screen_height) draw_end = screen_height - 1;

            // Determine which texture to use based on the map grid value.
            int texNum = map_grid.grid[ray_curr_grid_x][ray_curr_grid_y] - 1;

            // Calculate the exact point of wall hit (wallX) to determine the corresponding x-coordinate on the texture.
            double wallX; 
            if (side == 0) wallX = player.position.y + final_wall_dist * rayDir.y;
            else           wallX = player.position.x + final_wall_dist * rayDir.x;
            wallX -= floor(wallX); // Get only the fraction

            // Calculate the x-coordinate on the texture (texX) by multiplying wallX by the texture width and converting to an integer.
            int texX = (int)(wallX * (double)textureWidth);

            // Flip texture if we are looking at the "back" of the wall
            if(side == 0 && rayDir.x > 0) texX = textureWidth - texX - 1;
            if(side == 1 && rayDir.y < 0) texX = textureWidth - texX - 1;

            // Calculate the step size for texture coordinate interpolation
            double step = 1.0 * textureHeight / line_height;
            // Calculate the initial texture position
            double texPos = (draw_start - screen_height / 2 + line_height / 2) * step;

            // Iterate through the vertical slice of the wall
            for(int y = draw_start; y < draw_end; y++)
            {
                // Cast the texture position to an integer and mask it to stay within 0-63
                int texY = (int)texPos & (textureHeight - 1);
                texPos += step;

                // Get the color from the texture using texNum, texX, and texY.
                uint32_t color = texture[texNum][textureHeight * texY + texX];

                // Calculate fog effect based on distance. The farther the wall, the more it should blend with the fog color (e.g., gray).
                double fogDensity = 0.07; 
                double visibility = 1.0 / exp(final_wall_dist * fogDensity);

                // Clamp to ensure it stays between 0 and 1
                if (visibility > 1.0) visibility = 1.0;
                if (visibility < 0.0) visibility = 0.0;

                // Apply the fog effect by blending the wall color with a fog color (e.g., 0xFF777777) based on visibility.
                uint32_t fogColor = 0xFF777777;
                uint8_t r = ((color >> 16) & 0xFF) * visibility + ((fogColor >> 16) & 0xFF) * (1 - visibility);
                uint8_t g = ((color >> 8) & 0xFF) * visibility + ((fogColor >> 8) & 0xFF) * (1 - visibility);
                uint8_t b = (color & 0xFF) * visibility + (fogColor & 0xFF) * (1 - visibility);
                color = (0xFF << 24) | (r << 16) | (g << 8) | b;

                // Set the pixel color in the pixel buffer at (x, y) to the color obtained from the texture.
                sdl.pixelBuffer[y][x] = color;
            }

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