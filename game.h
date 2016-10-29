#ifndef __GAME__
#define __GAME__

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "globals.h"
#include "multiplayer.h"

#define MULTIPLAYER_MESSAGE_TYPE_GAME_START 0

typedef enum {MODE_CLASSIC, MODE_TIMED, MODE_MULTIPLAYER} game_mode;

typedef struct {
	SDL_Renderer* renderer;
	SDL_Window* window;
	TTF_Font* scoreFont;
	TTF_Font* endFont;
	TTF_Font* menuFont;
	TTF_Font* selectedMenuFont;
	TTF_Font* highScoreFont;
	TTF_Font* highScoreTitleFont;
	s_SocketConnection socketConnection;
	game_mode mode;
	Uint32 timeStarted;
	Uint32 timeFinished;
	int grid[HEIGHT_GRID][WIDTH_GRID];
	int colors[NB_COLORS][3];
	int iTurns;
	int iSelectedColor;
	int currentPlayerIndex;
	char canPlay;
	char receivedGrid;
} s_Game;

void game_init(s_Game *game);
void game_clean(s_Game *game);
void game_start(s_Game *game);
void game_restart(s_Game *game);
char game_checkBoard(s_Game* game);
char game_selectColor(s_Game* game);
void game_getNeighbours(int x, int y, int neighbours[4][2], int* nbNeighbours);
char game_is(s_Game *game, game_mode mode);
void game_setMode(s_Game* game, game_mode mode);
void game_finish(s_Game *game, const char won);
void game_getTimer(s_Game *game, char *timer);

#endif
