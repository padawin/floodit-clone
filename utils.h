#ifndef __UTILS__
#define __UTILS__

#include <SDL2/SDL_ttf.h>
#include "game.h"

int utils_popArray(int* array, int* arrayLength);
void utils_createTextTexture(
	SDL_Renderer *renderer,
	TTF_Font *font,
	const char* text,
	SDL_Color color,
	SDL_Texture **texture
);

#endif
