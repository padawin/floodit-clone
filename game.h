#ifndef __GAME__
#define __GAME__

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "globals.h"
#include "multiplayer.h"

#define FLAG_DONE 0x1
#define FLAG_NEEDS_RESTART 0x2

#define STATE_MAINMENU 1
#define STATE_PLAY 2
#define STATE_FINISH_WON 3
#define STATE_FINISH_LOST 4
#define STATE_HIGH_SCORES 5
#define STATE_MULTIPLAYER_SETUP 6

typedef enum {MODE_CLASSIC, MODE_TIMED} game_mode;

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
	int iState;
	int iSelectedColor;
	char cFlags;
} s_Game;

void game_init(s_Game *game);
void game_clean(s_Game *game);
void game_start(s_Game *game, game_mode mode);
void game_restart(s_Game *game);
char game_checkBoard(s_Game* game);
char game_selectColor(s_Game* game);
void game_getNeighbours(int x, int y, int neighbours[4][2], int* nbNeighbours);
char game_is(s_Game *game, char flag);
void game_setFlag(s_Game *game, char flag);
void game_unSetFlag(s_Game *game, char flag);
void game_finish(s_Game *game, const char won);
void game_getTimer(s_Game *game, char *timer);

#endif
