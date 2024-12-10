#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

//option#1 (without main parameters) SDL redefines main to SDL_main
// // #ifdef main
// // #undef main
// // #endif

// // int main() {
// //     std::cout << "Hello, World!" << std::endl;
// //     return 0;
// // }


// Screen dimensions
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// Paddle dimensions
const int PADDLE_WIDTH = 20;
const int PADDLE_HEIGHT = 100;

// Ball dimensions
const int BALL_SIZE = 20;

// Ball speed
int ballSpeedX = 5;
int ballSpeedY = 5;

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    // initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "Failed to initialize SDL_mixer: " << Mix_GetError() << std::endl;
        return -1;
    }

    // load sound effects
    Mix_Chunk* paddleSound = Mix_LoadWAV("res/sounds/paddle_hit.wav");
    Mix_Chunk* wallSound = Mix_LoadWAV("res/sounds/wall_hit.wav");
    Mix_Chunk* scoreSound = Mix_LoadWAV("res/sounds/score_update.wav");

    if (!paddleSound || !wallSound || !scoreSound) {    
        std::cerr << "Failed to load sound effects: " << Mix_GetError() << std::endl;
        Mix_CloseAudio();
        SDL_Quit();
        return -1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        Mix_FreeChunk(paddleSound);
        Mix_FreeChunk(wallSound);
        Mix_FreeChunk(scoreSound);
        SDL_Quit();
        return -1;
    }

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        Mix_FreeChunk(paddleSound);
        Mix_FreeChunk(wallSound);
        Mix_FreeChunk(scoreSound);
        Mix_CloseAudio();
        SDL_Quit();
        return -1;
    }

    // Paddles and ball
    SDL_Rect paddle1 = {50, (SCREEN_HEIGHT / 2) - (PADDLE_HEIGHT / 2), PADDLE_WIDTH, PADDLE_HEIGHT};
    SDL_Rect paddle2 = {SCREEN_WIDTH - 50 - PADDLE_WIDTH, (SCREEN_HEIGHT / 2) - (PADDLE_HEIGHT / 2), PADDLE_WIDTH, PADDLE_HEIGHT};
    SDL_Rect ball = {(SCREEN_WIDTH / 2) - (BALL_SIZE / 2), (SCREEN_HEIGHT / 2) - (BALL_SIZE / 2), BALL_SIZE, BALL_SIZE};

    bool running = true;
    SDL_Event event;

    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Keyboard state
        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        if (keystate[SDL_SCANCODE_W] && paddle1.y > 0) paddle1.y -= 5;
        if (keystate[SDL_SCANCODE_S] && paddle1.y < SCREEN_HEIGHT - PADDLE_HEIGHT) paddle1.y += 5;
        if (keystate[SDL_SCANCODE_UP] && paddle2.y > 0) paddle2.y -= 5;
        if (keystate[SDL_SCANCODE_DOWN] && paddle2.y < SCREEN_HEIGHT - PADDLE_HEIGHT) paddle2.y += 5;

        // Move ball
        ball.x += ballSpeedX;
        ball.y += ballSpeedY;

        // Ball collision with top and bottom
        if (ball.y <= 0 || ball.y + BALL_SIZE >= SCREEN_HEIGHT) {
            ballSpeedY = -ballSpeedY;
        }

        // Ball collision with paddles
        if (SDL_HasIntersection(&ball, &paddle1) || SDL_HasIntersection(&ball, &paddle2)) {
            ballSpeedX = -ballSpeedX;
            // ballSpeedY = +ballSpeedY;
            Mix_PlayChannel(-1, paddleSound, 0);
        }

        // Ball goes off screen (reset position)
        if (ball.x <= 0 || ball.x + BALL_SIZE >= SCREEN_WIDTH) {
            ball.x = (SCREEN_WIDTH / 2) - (BALL_SIZE / 2);
            ball.y = (SCREEN_HEIGHT / 2) - (BALL_SIZE / 2);
            ballSpeedX = -ballSpeedX;
            // ballSpeedX = -ballSpeedY;
           Mix_PlayChannel(-1, wallSound, 0);
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw paddles and ball
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &paddle1);
        SDL_RenderFillRect(renderer, &paddle2);
        SDL_RenderFillRect(renderer, &ball);

        // Update screen
        SDL_RenderPresent(renderer);

        // Delay to control frame rate
        SDL_Delay(16);
    }

    // Clean up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_FreeChunk(paddleSound);
    Mix_FreeChunk(wallSound);
    Mix_FreeChunk(scoreSound);
    Mix_CloseAudio();    
    SDL_Quit();

    return 0;
}
