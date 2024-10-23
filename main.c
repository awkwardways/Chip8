#include <stdio.h>
#include "include/chip8.h"
#include <SDL2/SDL.h>

#undef EXIT_SUCCESS
#undef EXIT_FAILURE

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1


int initializeSDL(SDL_Window** window, SDL_Renderer** renderer, SDL_Texture** windowTexture) {
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		printf("Could not initialize SDL. ERROR: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}
	
	*window = SDL_CreateWindow("Chip 8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 480, 320, 0);
	if(!(*window)) {
		printf("Could not create window. ERROR: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	*renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
	if(!(*renderer)) {
		printf("Could not create renderer. ERROR: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	*windowTexture = SDL_CreateTexture(
		*renderer, 
		SDL_PIXELFORMAT_RGBA8888, 
		SDL_TEXTUREACCESS_STREAMING, 
		CHIP_8_SCREEN_WIDTH, 
		CHIP_8_SCREEN_HEIGHT);
	if(!(*windowTexture)) {
		printf("Could not create window texture. ERROR: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

void quitSDL(SDL_Window** window, SDL_Renderer** renderer) {
	SDL_DestroyWindow(*window);
	SDL_DestroyRenderer(*renderer);
	SDL_Quit();
}

int main(int argc, char* argv[]) {
	SDL_Renderer* renderer       = NULL;
	SDL_Window*   window         = NULL;
	SDL_Texture*  windowTexture  = NULL;
	if(initializeSDL(&window, &renderer, &windowTexture)) {
		return EXIT_FAILURE;
	}

	initC8();

  if(loadC8(argv[1])) {
		quitSDL(&window, &renderer);
		return EXIT_FAILURE;
	}

	int run = 1;
	SDL_Event event;
	void* pixels;
	int pitch;
	uint64_t prev = SDL_GetTicks64() + 16;
	while(run) {
		if(SDL_GetTicks64() > prev) {
			prev = SDL_GetTicks64() + 16;
			if(delayTimer > 0) delayTimer--;
			if(soundTimer > 0) soundTimer--;
		}

		while(SDL_PollEvent(&event)) {
			if(event.type == SDL_QUIT) run = 0;
			else if(event.type == SDL_KEYDOWN) {
				switch(event.key.keysym.sym) {
					case SDLK_0:
					case SDLK_1:
					case SDLK_2:
					case SDLK_3:
					case SDLK_4:
					case SDLK_5:
					case SDLK_6:
					case SDLK_7:
					case SDLK_8:
					case SDLK_9:
						keyPressed = event.key.keysym.sym - 48;
					break;

					case SDLK_a:
					case SDLK_b:
					case SDLK_c:
					case SDLK_d:
					case SDLK_e:
					case SDLK_f:
						keyPressed = event.key.keysym.sym - 87;
					break;

					case SDLK_ESCAPE:
						run = 0;
					break;
				}
			}
			else if(event.type == SDL_KEYUP) {
				keyPressed = 16;
			}
		}

		if(drawFlag) {
			SDL_LockTexture(windowTexture, NULL, &pixels, &pitch);
			memcpy((uint8_t*)pixels, newScreen, sizeof(newScreen));
			SDL_UnlockTexture(windowTexture);
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, windowTexture, NULL, NULL);
			SDL_RenderPresent(renderer);
			drawFlag = 0;
		} 

		cycle();
	}
	quitSDL(&window, &renderer);
	return EXIT_SUCCESS;
}
