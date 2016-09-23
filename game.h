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

void generateGrid(s_Game* game);
char checkBoard(s_Game* game);
char selectColor(s_Game* game);
void getNeighbours(int x, int y, int neighbours[4][2], int* nbNeighbours);

#endif
