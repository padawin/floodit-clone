#ifndef __GAME__
#define __GAME__

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "globals.h"

#define FLAG_DONE 0x1
#define FLAG_NEEDS_REFRESH 0x2
#define FLAG_NEEDS_RESTART 0x4

#define STATE_MAIN_MENU 1
#define STATE_PLAY 2
#define STATE_FINISH_WON 3
#define STATE_FINISH_LOST 4

typedef struct {
	SDL_Renderer* renderer;
	SDL_Window* window;
	TTF_Font* font;
	int grid[HEIGHT_GRID][WIDTH_GRID];
	int colors[NB_COLORS][3];
	int iTurns;
	int iState;
	int iSelectedColor;
	char cFlags;
} s_Game;

void game_init(s_Game *game);
void game_generateGrid(s_Game* game);
char game_checkBoard(s_Game* game);
char game_selectColor(s_Game* game);
void game_getNeighbours(int x, int y, int neighbours[4][2], int* nbNeighbours);
char game_is(s_Game *game, char flag);
void game_setFlag(s_Game *game, char flag);
void game_unSetFlag(s_Game *game, char flag);

#endif
