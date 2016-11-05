#ifndef __GAME__
#define __GAME__

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "globals.h"
#include "multiplayer.h"

#define MULTIPLAYER_MESSAGE_TYPE_GAME_START 0
#define MULTIPLAYER_MESSAGE_TYPE_GRID 2
#define MULTIPLAYER_MESSAGE_TYPE_PLAYER_TURN 3
#define MULTIPLAYER_MESSAGE_TYPE_PLAYER_END_TURN 4
#define MULTIPLAYER_MESSAGE_TYPE_PLAYER_LOST 5

typedef enum {MODE_CLASSIC, MODE_TIMED, MODE_MULTIPLAYER} game_mode;
typedef enum {
	CLIENT_PLAYED,
	INVALID_PLAY,
	GAME_WON,
	GAME_LOST,
	END_TURN
} game_play_result;

typedef struct {
	int color;
	char owner;
} s_GridCell;

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
	s_GridCell grid[HEIGHT_GRID][WIDTH_GRID];
	int colors[NB_COLORS][3];
	int iTurns;
	int iSelectedColor;
	int currentPlayerIndex;
	char canPlay;
	char lost;
	char receivedGrid;
} s_Game;

// Game flow
void game_init(s_Game *game);
void game_start(s_Game *game);
void game_restart(s_Game *game);
void game_finish(s_Game *game, const char won);
void game_clean(s_Game *game);
game_play_result game_play(s_Game *game, int selectedColor);

// Game attributes
char game_is(s_Game *game, game_mode mode);
void game_setMode(s_Game* game, game_mode mode);
void game_getTimer(s_Game *game, char *timer);
int game_getGridCellColor(s_Game *game, int x, int y);
void game_setGridCellColor(s_Game *game, int x, int y, int color);

// Board manipulation and analysis
char game_selectColor(s_Game* game, int color);
void game_getNeighbours(int x, int y, int neighbours[4][2], int* nbNeighbours);
void game_setGrid(s_Game* game, s_TCPpacket packet);

// Multiplayer
char game_processIncomingPackets(s_Game *game);


#endif
