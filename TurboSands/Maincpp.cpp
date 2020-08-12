#include <iostream>

#include <SDL.h>
#include <SDL_ttf.h>

// FPS calculations for FPS display
#include "fps.h"

using namespace std;

#define EMPTY_PIXEL_VALUE (255<<24 | 255<<16 | 255<<8 | 255)
#define SAND_PIXEL_VALUE 0

/*
* Helpler function to draw some text on the SDL renderer
*/
void PutText(char* text, TTF_Font* font, SDL_Renderer* renderer)
{
	SDL_Color textColor = { 0, 0, 255, 0 };
	SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, textColor);
	SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	int text_width = textSurface->w;
	int text_height = textSurface->h;
	SDL_FreeSurface(textSurface);
	SDL_Rect renderQuad = { 20, 30, text_width, text_height };
	SDL_RenderCopy(renderer, textTexture, NULL, &renderQuad);
	SDL_DestroyTexture(textTexture);
}

void StepSimulation(Uint32* pixels)
{
	for (int y = 480-1; y >= 0 ; y--)
	{
		for (int x = 0; x < 640; x++)
		{
			if (pixels[y * 640 + x] == EMPTY_PIXEL_VALUE)
				continue;

			Uint32 empty = EMPTY_PIXEL_VALUE;
			bool canDrop = y == 480 ? false : (pixels[(y+1) * 640 + x] == EMPTY_PIXEL_VALUE);
			bool canRollLeft = y == 480 ? false :
				(x==0 ? false : (pixels[(y+1) * 640 + (x-1)] == EMPTY_PIXEL_VALUE));
			bool canRollRight = y == 480 ? false :
				(x == 640-2 ? false : (pixels[(y+1) * 640 + (x+1)] == EMPTY_PIXEL_VALUE));
			
			if (canDrop)
			{
				pixels[y * 640 + x] = EMPTY_PIXEL_VALUE;
				pixels[(y + 1) * 640 + x] = SAND_PIXEL_VALUE;
			}
			else if (canRollLeft || canRollRight)
			{
				// If both, take randomly only other
				if (canRollLeft && canRollRight)
				{
					if (rand() % 1 == 0)
						canRollLeft = false;
					else
						canRollRight = false;
				}

				if (canRollLeft)
				{
					pixels[y * 640 + x] = EMPTY_PIXEL_VALUE;
					pixels[(y + 1) * 640 + (x - 1)] = SAND_PIXEL_VALUE;
				}
				if (canRollRight)
				{
					pixels[y * 640 + x] = EMPTY_PIXEL_VALUE;
					pixels[(y + 1) * 640 + (x + 1)] = SAND_PIXEL_VALUE;
				}
			}
			else
			{
				// NOP
			}
		}
	}
}


int main(int argc, char ** argv)
{
	bool sim_running = false;
	bool quit = false;
	SDL_Event event;
	char FPSsbuf[64];
	FPSsbuf[0] = ' ';
	FPSsbuf[1] = '\0';

	FPS_init();
	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();
	TTF_Font* font = TTF_OpenFont("Topaz_a500.ttf", 24); //this opens a font style and sets a size
	cout << TTF_GetError();

	SDL_Window* window = SDL_CreateWindow("SDL2 Turbosands",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

	SDL_Texture* texture = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 640, 480);

	Uint32* pixels = new Uint32[640 * 480];
	memset(pixels, 255, 640 * 480 * sizeof(Uint32));

	for (int y = 0; y < 480; y++)
	{
		for (int x = 0; x < 640; x++)
		{
			if (rand() % 10 == 0)
				pixels[y * 640 + x] = SAND_PIXEL_VALUE;
		}
	}

	while (!quit)
	{
		// Add a random sand particles
		if (sim_running)
		{
			for (int i = 0; i < 50; i++)
			{
				int randomY = rand() % 480;
				int randomX = rand() % 640;
				pixels[randomY * 640 + randomX] = 0;
			}
			StepSimulation(pixels);
		}
		SDL_UpdateTexture(texture, NULL, pixels, 640 * sizeof(Uint32));

		int hasEvent = SDL_PollEvent(&event);
		if (hasEvent)
		{
			switch (event.type)
			{
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYUP:
				if (event.key.keysym.sym == SDLK_r)
					sim_running = !sim_running;
				break;
			}
		}

		
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		sprintf_s(FPSsbuf, sizeof FPSsbuf, "%d", (int)FPS_think());
		PutText(FPSsbuf, font, renderer);

		SDL_RenderPresent(renderer);
	}
	
	delete[] pixels;
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);

	SDL_DestroyWindow(window);
	TTF_Quit();
	SDL_Quit();

	return 0;
}