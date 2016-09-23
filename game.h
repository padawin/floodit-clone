#ifndef __GAME__
#define __GAME__

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "globals.h"

typedef struct {
	SDL_Renderer* renderer;
	SDL_Window* window;
	TTF_Font* font;
	int grid[HEIGHT_GRID][WIDTH_GRID];
	int colors[NB_COLORS][3];
	int iTurns;
	int iState;
	int iSelectedColor;
} s_Game;

void game_generateGrid(s_Game* game);
char game_checkBoard(s_Game* game);
char game_selectColor(s_Game* game);
void game_getNeighbours(int x, int y, int neighbours[4][2], int* nbNeighbours);

#endif
