#ifndef __GAME__
#define __GAME__

typedef struct {
	SDL_Renderer* renderer;
	SDL_Window* window;
	TTF_Font* font;
	int grid[HEIGHT_GRID][WIDTH_GRID];
	int iTurns;
	int iState;
	int iSelectedColor;
} s_Game;

#endif
