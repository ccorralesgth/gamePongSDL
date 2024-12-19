#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <cmath>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;
const int PADDLE_WIDTH = 20;
const int PADDLE_HEIGHT = 100;
const int BALL_SIZE = 20;
const int MAX_SCORE = 3;

void renderText(SDL_Renderer *renderer, TTF_Font *font, const std::string &text, int x = 0, int y = 0, bool centeredX = true, bool centeredY = true)
{
	SDL_Color color = {255, 255, 255, 255}; // white
	SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), color);
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_Rect dstRect = {};

	if (centeredX)
	{
		if (x == 0) // if center is true, x become an offset from the center x point
			x = (SCREEN_WIDTH / 2 - surface->w / 2);
		else
			x = (SCREEN_WIDTH / 2 - surface->w / 2) + x;
	}
	if (centeredY)
	{
		if (y == 0) // if center is true, y become an offset from the center y point
			y = (SCREEN_HEIGHT / 2 - surface->h / 2);
		else
			y = (SCREEN_HEIGHT / 2 - surface->h / 2) + y;
	}

	dstRect = {x, y, surface->w, surface->h};

	SDL_RenderCopy(renderer, texture, NULL, &dstRect);
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
}
void renderBorder(SDL_Renderer *renderer, int thickness)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White color
    for (int i = 0; i < thickness; ++i)
    {
        SDL_RenderDrawLine(renderer, i, i, SCREEN_WIDTH - i, i);                                         // Top border
        SDL_RenderDrawLine(renderer, i, i, i, SCREEN_HEIGHT - i);                                        // Left border
        SDL_RenderDrawLine(renderer, SCREEN_WIDTH - 1 - i, i, SCREEN_WIDTH - 1 - i, SCREEN_HEIGHT - i);  // Right border
        SDL_RenderDrawLine(renderer, i, SCREEN_HEIGHT - 1 - i, SCREEN_WIDTH - i, SCREEN_HEIGHT - 1 - i); // Bottom border
    }
}
void renderDashedLine(SDL_Renderer *renderer, int x, int y1, int y2, int dashLength, int gapLength)
{
    bool draw = true;
    for (int y = y1; y < y2; y += dashLength + gapLength)
    {
        if (draw)
        {
            SDL_RenderDrawLine(renderer, x, y, x, y + dashLength);
        }
        draw = !draw;
    }
}

bool Initialize()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
		SDL_Log("Failed to initialize SDL: %s\n", SDL_GetError());
		return false;
	}

	if (TTF_Init() == -1)
	{
		std::cerr << "Failed to initialize SDL_ttf: " << TTF_GetError() << std::endl;
		SDL_Quit();
		return false;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		std::cerr << "Failed to initialize SDL_mixer: " << Mix_GetError() << std::endl;
		TTF_Quit();
		SDL_Quit();
		return false;
	}

	return true;
}

void Close()
{
	Mix_CloseAudio();
	TTF_Quit();
	SDL_Quit();
}

int main(int argc, char *argv[])
{
	SDL_Window *window = nullptr;
	SDL_Renderer *renderer = nullptr;

	// Initialize libraries (SDL, Mixer, TTF)
	if (!Initialize())
	{
		Close();
		return -1;
	}
	else
	{
		// create window
		window = SDL_CreateWindow("Pong", 
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 
		// SDL_WINDOW_SHOWN | SDL_WINDOW_MAXIMIZED);
		SDL_WINDOW_FULLSCREEN);
		if (!window)
		{
			std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
			Close();
			return -1;
		}

		// create render
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		if (!renderer)
		{
			std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
			SDL_DestroyWindow(window);
			Close();
			return -1;
		}

		// Load font
		TTF_Font *font = TTF_OpenFont("res/fonts/NES-Chimera/NES-Chimera.ttf", 24);
		if (!font)
		{
			std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
			SDL_DestroyRenderer(renderer);
			SDL_DestroyWindow(window);
			Close();
			return -1;
		}

		// show main window
		bool start = false;
		while (!start)
		{
			SDL_Event e;
			while (SDL_PollEvent(&e) != 0)
			{
				if (e.type == SDL_QUIT)
				{
					SDL_DestroyRenderer(renderer);
					SDL_DestroyWindow(window);
					Close();
					return 0;
				}
				else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN)
				{
					start = true;
				}
				else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
				{
					SDL_DestroyRenderer(renderer);
					SDL_DestroyWindow(window);
					Close();
					return 0;
				}
			}
			// Clear screen
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
			SDL_RenderClear(renderer);
			renderText(renderer, font, "Press Enter to Start");
			SDL_RenderPresent(renderer);
		}
		// Load sound effects
		Mix_Chunk *paddleSound = Mix_LoadWAV("res/sounds/paddle_hit.wav");
		Mix_Chunk *wallSound = Mix_LoadWAV("res/sounds/wall_hit.wav");
		Mix_Chunk *scoreSound = Mix_LoadWAV("res/sounds/score_update.wav");

		if (!paddleSound || !wallSound || !scoreSound)
		{
			std::cerr << "Failed to load sound effects: " << Mix_GetError() << std::endl;
			Mix_CloseAudio();
			TTF_Quit();
			SDL_Quit();
			return -1;
		}

		SDL_Rect paddle1 = {50, (SCREEN_HEIGHT / 2) - (PADDLE_HEIGHT / 2), PADDLE_WIDTH, PADDLE_HEIGHT};
		SDL_Rect paddle2 = {SCREEN_WIDTH - 50 - PADDLE_WIDTH, (SCREEN_HEIGHT / 2) - (PADDLE_HEIGHT / 2), PADDLE_WIDTH, PADDLE_HEIGHT};

		SDL_Rect ball = {(SCREEN_WIDTH / 2) - (BALL_SIZE / 2), (SCREEN_HEIGHT / 2) - (BALL_SIZE / 2), BALL_SIZE, BALL_SIZE};
		int ballSpeedX = 5;
		int ballSpeedY = 5;

		int leftScore = 0;
		int rightScore = 0;

		// Game loop
		SDL_Event event;
		bool running = true;
		bool gameOver = false;
		bool pause = false;
		while (running)
		{
			while (SDL_PollEvent(&event) != 0)
			{
				if (event.type == SDL_QUIT)
				{
					running = false;
				}
				else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN)
				{
					if (pause)
					{
						pause = false;
					}
					if (gameOver)
					{
						leftScore = 0;
						rightScore = 0;
						ball.x = (SCREEN_WIDTH / 2) - (BALL_SIZE / 2);
						ball.y = (SCREEN_HEIGHT / 2) - (BALL_SIZE / 2);
						ballSpeedX = 5;
						ballSpeedY = 5;
						gameOver = false;
					}
				}
				else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
				{
					running = false;
				}
				else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_p)
				{
					pause = true;
				}
			}
			if (pause)
			{
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
				SDL_RenderClear(renderer);
				renderText(renderer, font, "Game Paused");
				SDL_RenderPresent(renderer);
				continue;
			}
			if (gameOver)
			{
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
				SDL_RenderClear(renderer);
				std::string winner = "";
				if (leftScore > rightScore)				
					winner = "Left Player Wins";				
				else
					winner = "Right Player Wins";
				
				renderText(renderer, font, winner, 0, (SCREEN_HEIGHT / 2) - 50, true, false);
				renderText(renderer, font, "Press Enter to Restart");
				renderText(renderer, font, "Press ESC to Quit", 0, (SCREEN_HEIGHT / 2) + 20, true, false);

				SDL_RenderPresent(renderer);
				continue;
			}
			else
			{
				// check if winner
				if (rightScore >= MAX_SCORE || leftScore >= MAX_SCORE)
				{
					gameOver = true;
					continue;
				}

				// Keyboard state
				const Uint8 *keystate = SDL_GetKeyboardState(NULL);
				if (keystate[SDL_SCANCODE_W] && paddle1.y > 0)
					paddle1.y -= 5;
				if (keystate[SDL_SCANCODE_S] && paddle1.y < SCREEN_HEIGHT - PADDLE_HEIGHT)
					paddle1.y += 5;
				if (keystate[SDL_SCANCODE_UP] && paddle2.y > 0)
					paddle2.y -= 5;
				if (keystate[SDL_SCANCODE_DOWN] && paddle2.y < SCREEN_HEIGHT - PADDLE_HEIGHT)
					paddle2.y += 5;

				// move ball
				ball.x += ballSpeedX;
				ball.y += ballSpeedY;

				// Ball collision with top and bottom
				if (ball.y <= 0 || ball.y + BALL_SIZE >= SCREEN_HEIGHT)
				{
					ballSpeedY = -ballSpeedY;
					Mix_PlayChannel(-1, wallSound, 0);
				}

				// Ball collision with paddles
				if (SDL_HasIntersection(&ball, &paddle1) || SDL_HasIntersection(&ball, &paddle2))
				{
					ballSpeedX = -ballSpeedX;
					Mix_PlayChannel(-1, paddleSound, 0);
				}

				if (ball.x <= 0)
				{
					rightScore++;
					ball.x = (SCREEN_WIDTH / 2) - (BALL_SIZE / 2);
					ball.y = (SCREEN_HEIGHT / 2) - (BALL_SIZE / 2);
					ballSpeedX = -ballSpeedX;
					Mix_PlayChannel(-1, scoreSound, 0);
				}
				else if (ball.x + BALL_SIZE >= SCREEN_WIDTH)
				{
					leftScore++;
					ball.x = (SCREEN_WIDTH / 2) - (BALL_SIZE / 2);
					ball.y = (SCREEN_HEIGHT / 2) - (BALL_SIZE / 2);
					ballSpeedX = -ballSpeedX;
					Mix_PlayChannel(-1, scoreSound, 0);
				}

				// Clear screen
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // black color
				SDL_RenderClear(renderer);

				renderText(renderer, font, std::to_string(leftScore), 15, 15 ,false, false );
				renderText(renderer, font, std::to_string(rightScore), SCREEN_WIDTH - 40, 15, false,false);

				renderBorder(renderer, 5);            	
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White color	
				renderDashedLine(renderer, SCREEN_WIDTH / 2, 0, SCREEN_HEIGHT, 10, 10);			
								
				SDL_RenderFillRect(renderer, &ball);
				SDL_RenderFillRect(renderer, &paddle1);
				SDL_RenderFillRect(renderer, &paddle2);
				SDL_RenderPresent(renderer);

				SDL_Delay(SCREEN_TICKS_PER_FRAME);
			}
		}
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	Close();
	return 0;
}
