#include <time.h>
#include "game.h"
#include "utils.h"
#include "high_score.h"
#include "multiplayer.h"

char _spreadColor(s_Game *game, int selectedColor, int startX, int startY);
void generateGrid(s_Game* game);
void _generateFirstPlayer(s_Game *game);

int g_startPositionPlayers[4][2] = {
	{0, 0},
	{WIDTH_GRID - 1, HEIGHT_GRID - 1},
	{WIDTH_GRID - 1, 0},
	{0, HEIGHT_GRID - 1}
};

void game_init(s_Game *game) {
	game->scoreFont = TTF_OpenFont("ClearSans-Medium.ttf", 18);
	game->endFont = TTF_OpenFont("ClearSans-Medium.ttf", 18);
	game->menuFont = TTF_OpenFont("ClearSans-Medium.ttf", 18);
	game->selectedMenuFont = TTF_OpenFont("ClearSans-Medium.ttf", 24);
	game->highScoreTitleFont = TTF_OpenFont("ClearSans-Medium.ttf", 24);
	game->highScoreFont = TTF_OpenFont("ClearSans-Medium.ttf", 18);
	game->colors[0][0] = 255;
	game->colors[0][1] = 0;
	game->colors[0][2] = 0;
	game->colors[1][0] = 0;
	game->colors[1][1] = 255;
	game->colors[1][2] = 0;
	game->colors[2][0] = 0;
	game->colors[2][1] = 0;
	game->colors[2][2] = 255;
	game->colors[3][0] = 255;
	game->colors[3][1] = 255;
	game->colors[3][2] = 0;
	game->colors[4][0] = 255;
	game->colors[4][1] = 0;
	game->colors[4][2] = 255;
	game->colors[5][0] = 0;
	game->colors[5][1] = 255;
	game->colors[5][2] = 255;
	game->mode = MODE_CLASSIC;
	game->canPlay = 0;
	game->receivedGrid = 0;
}

void game_clean(s_Game *game) {
	TTF_CloseFont(game->scoreFont);
	game->scoreFont = NULL;
	TTF_CloseFont(game->endFont);
	game->endFont = NULL;
	TTF_CloseFont(game->menuFont);
	game->menuFont = NULL;
	TTF_CloseFont(game->selectedMenuFont);
	game->selectedMenuFont = NULL;
	TTF_CloseFont(game->highScoreFont);
	game->highScoreFont = NULL;
	TTF_CloseFont(game->highScoreTitleFont);
	game->highScoreTitleFont = NULL;

	if (game_is(game, MODE_MULTIPLAYER)) {
		multiplayer_clean(&game->socketConnection);
	}
}

void game_start(s_Game *game) {
	game->timeStarted = 0;
	game->timeFinished = 0;

	if (game_is(game, MODE_TIMED)) {
		game->timeStarted = SDL_GetTicks();
	}

	char isMultiplayer = game_is(game, MODE_MULTIPLAYER);
	game->canPlay = 0;
	if (!isMultiplayer || (isMultiplayer && game->socketConnection.type == SERVER)) {
		generateGrid(game);

		if (isMultiplayer) {
			//send grid to players
			game_broadcastGrid(game);
		}

		_generateFirstPlayer(game);

		if (game->currentPlayerIndex == 0) {
			game->canPlay = 1;
		}
		else {
			// notify first player
			s_TCPpacket packet;
			packet.type = MULTIPLAYER_MESSAGE_TYPE_PLAYER_TURN;
			packet.size = 0;
			multiplayer_send_message(
				game->socketConnection,
				game->currentPlayerIndex - 1,
				packet
			);
		}
	}

	// program main loop
	game->iSelectedColor = 0;
	game->iTurns = 1;
}

void game_broadcastGrid(s_Game *game) {
	s_TCPpacket packet;
	packet.type = MULTIPLAYER_MESSAGE_TYPE_GRID;
	packet.size = 196;
	int i, j;

	for (j = 0; j < HEIGHT_GRID; ++j) {
		for (i = 0; i < WIDTH_GRID; ++i) {
			packet.data[j * WIDTH_GRID + i] = game->grid[j][i];
		}
	}

	multiplayer_broadcast(game->socketConnection, packet);
}

void _generateFirstPlayer(s_Game *game) {
	if (!game_is(game, MODE_MULTIPLAYER)) {
		game->currentPlayerIndex = 0;
	}
	else {
		time_t t;
		srand((unsigned) time(&t));
		game->currentPlayerIndex = rand() % (game->socketConnection.nbConnectedSockets + 1);
	}
}

void game_restart(s_Game *game) {
	game_start(game);
}

void generateGrid(s_Game* game) {
	int i, j;
	time_t t;

	srand((unsigned) time(&t));
	for (j = 0; j < HEIGHT_GRID; ++j) {
		for (i = 0; i < WIDTH_GRID; ++i) {
			game->grid[j][i] = rand() % NB_COLORS;
		}
	}

	game->receivedGrid = 1;
}

void game_setGrid(s_Game* game, s_TCPpacket packet) {
	int i, j;
	for (j = 0; j < HEIGHT_GRID; ++j) {
		for (i = 0; i < WIDTH_GRID; ++i) {
			game->grid[j][i] = packet.data[j * WIDTH_GRID + i];
		}
	}

	game->receivedGrid = 1;
}

char game_checkBoard(s_Game* game) {
	signed char color = -1;
	int i, j;
	for (j = 0; j < HEIGHT_GRID; ++j) {
		for (i = 0; i < WIDTH_GRID; ++i) {
			if (color != -1 && game->grid[j][i] != color) {
				return 0;
			}
			else {
				color = game->grid[j][i];
			}
		}
	}

	return 1;
}

char game_selectColor(s_Game* game, int color) {
	if (game_is(game, MODE_MULTIPLAYER) && game->socketConnection.type == CLIENT) {
		s_TCPpacket packet;
		packet.type = MULTIPLAYER_MESSAGE_TYPE_PLAYER_TURN;
		packet.size = 1;
		packet.data[0] = game->iSelectedColor;
		multiplayer_send_message(game->socketConnection, -1, packet);
		return -1;
	}

	int startX, startY;
	startX = g_startPositionPlayers[game->currentPlayerIndex][0];
	startY = g_startPositionPlayers[game->currentPlayerIndex][1];
	char ret = _spreadColor(game, color, startX, startY);

	return ret;
}

char _spreadColor(s_Game *game, int selectedColor, int startX, int startY) {
	char toVisitFlag = 0x1,
		 visitedFlag = 0x2;
	int i, j, nbToVisit, oldColor;
	int *toVisit;
	int **visited;

	oldColor = game->grid[startY][startX];
	if (selectedColor == oldColor) {
		return 0;
	}

	visited = (int **) malloc(HEIGHT_GRID * sizeof(int *));
	for (i = 0; i < HEIGHT_GRID; i++) {
		visited[i] = (int *) malloc(WIDTH_GRID * sizeof(int));
	}

	toVisit = (int *) malloc(WIDTH_GRID * HEIGHT_GRID * sizeof(int *));

	for (j = 0; j < HEIGHT_GRID; ++j) {
		for (i = 0; i < WIDTH_GRID; ++i) {
			visited[j][i] = 0;
			toVisit[j * WIDTH_GRID + i] = 0;
		}
	}

	toVisit[0] = startY * WIDTH_GRID + startX;
	visited[startY][startX] |= toVisitFlag;
	nbToVisit = 1;

	while (nbToVisit > 0) {
		int x, y, next = utils_popArray(toVisit, &nbToVisit);

		x = next % WIDTH_GRID;
		y = next / WIDTH_GRID;
		visited[y][x] |= visitedFlag;
		game->grid[y][x] = selectedColor;

		int neighbours[4][2];
		int nbNeighbours;
		game_getNeighbours(x, y, neighbours, &nbNeighbours);
		for (i = 0; i < nbNeighbours; ++i) {
			if (
				visited[neighbours[i][1]][neighbours[i][0]] == 0
				&& game->grid[neighbours[i][1]][neighbours[i][0]] == oldColor
			) {
				toVisit[nbToVisit++] = neighbours[i][1] * WIDTH_GRID + neighbours[i][0];
				visited[neighbours[i][1]][neighbours[i][0]] = toVisitFlag;
			}
		}
	}

	for (i = 0; i < HEIGHT_GRID; i++) {
		free(visited[i]);
	}
	free(visited);
	free(toVisit);

	return 1;
}

void game_getNeighbours(int x, int y, int neighbours[4][2], int* nbNeighbours) {
	(*nbNeighbours) = 0;
	if (x > 0) {
		neighbours[*nbNeighbours][0] = x - 1;
		neighbours[*nbNeighbours][1] = y;
		(*nbNeighbours) += 1;
	}

	if (x < WIDTH_GRID - 1) {
		neighbours[*nbNeighbours][0] = x + 1;
		neighbours[*nbNeighbours][1] = y;
		(*nbNeighbours) += 1;
	}

	if (y > 0) {
		neighbours[*nbNeighbours][0] = x;
		neighbours[*nbNeighbours][1] = y - 1;
		(*nbNeighbours) += 1;
	}

	if (y < HEIGHT_GRID - 1) {
		neighbours[*nbNeighbours][0] = x;
		neighbours[*nbNeighbours][1] = y + 1;
		(*nbNeighbours) += 1;
	}
}

char game_is(s_Game *game, game_mode mode) {
	return game->mode == mode;
}

void game_setMode(s_Game *game, game_mode mode) {
	game->mode = mode;
}

void game_finish(s_Game *game, const char won) {
	if (game_is(game, MODE_TIMED)) {
		game->timeFinished = SDL_GetTicks();
		if (won) {
			high_score_save(game->timeFinished - game->timeStarted, game->iTurns);
		}
	}
}

void game_getTimer(s_Game *game, char *timer) {
	Uint32 seconds = 0,
		minutes = 0,
		totalSeconds,
		endTime;

	if (game->timeFinished == 0) {
		endTime = SDL_GetTicks();
	}
	else {
		endTime = game->timeFinished;
	}

	totalSeconds = (endTime - game->timeStarted) / 1000;
	seconds = totalSeconds % 60;
	minutes = totalSeconds / 60;
	snprintf(timer, 6, "%02d:%02d", minutes, seconds);
}
