#include <SDL2/SDL.h>

#define SCALE 3

enum {
	SCREEN_WIDTH = 160,
	SCREEN_HEIGHT = 144,
};

int
main(void)
{
	if (SDL_Init(SDL_INIT_VIDEO)) {
		fprintf(stderr, "unable to init SDL: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Window *win = SDL_CreateWindow("Space Invaders", 0, 0, SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, SDL_WINDOW_SHOWN);

	if (win == NULL) {
		fprintf(stderr, "unable to create sdl win: %s\n", SDL_GetError());
		return 1;
	}

	getchar();

	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}
