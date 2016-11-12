#include <time.h>
#include "game.h"
#include "utils.h"
#include "high_score.h"
#include "multiplayer.h"

char _spreadColor(s_Game *game, int selectedColor, int startX, int startY, char init);
void _generateGrid(s_Game* game);
void _generateFirstPlayer(s_Game *game);
void _notifyServerPlayerTurn(s_Game *game);
char _processServerPackets(s_Game *game);
char _processClientPackets(s_Game *game);
void _setRotatedGridPacket(s_Game *game, s_TCPpacket *packet, int rotationMatrix[2][2], int shift[2]);
void _broadcastGrid(s_Game *game);
void _notifyCurrentPlayerTurn(s_Game *game, char isTurn);
void _selectNextPlayer(s_Game *game);
char _checkBoard(s_Game* game);
void _setPlayersInitialPosition(s_Game *game);
void _notifyCapturedPlayers(s_Game *game);
int _getGridCellOwner(s_Game *game, int x, int y);
void _setGridCellOwner(s_Game *game, int x, int y, int owner);
char _hasWinner(s_Game *game);
void _notifyWinner(s_Game *game);

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

void game_start(s_Game *game) {
	game->timeStarted = 0;
	game->timeFinished = 0;

	if (game_is(game, MODE_TIMED)) {
		game->timeStarted = SDL_GetTicks();
	}

	char isMultiplayer = game_is(game, MODE_MULTIPLAYER);
	game->canPlay = 0;
	if (!isMultiplayer || (isMultiplayer && game->socketConnection.type == SERVER)) {
		_generateGrid(game);

		if (isMultiplayer) {
			game->lost = 0;
			_setPlayersInitialPosition(game);
			//send grid to players
			_broadcastGrid(game);
		}

		_generateFirstPlayer(game);

		if (game->currentPlayerIndex == 0) {
			game->canPlay = 1;
		}
		else {
			// notify first player
			_notifyCurrentPlayerTurn(
				game,
				MULTIPLAYER_MESSAGE_TYPE_PLAYER_TURN
			);
		}
	}

	// program main loop
	game->iSelectedColor = 0;
	game->iTurns = 1;
}

void game_restart(s_Game *game) {
	game_start(game);
}

void game_finish(s_Game *game, const char won) {
	if (game_is(game, MODE_TIMED)) {
		game->timeFinished = SDL_GetTicks();
		if (won) {
			high_score_save(game->timeFinished - game->timeStarted, game->iTurns);
		}
	}
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

game_play_result game_play(s_Game *game, int selectedColor) {
	char isMultiplayer = game_is(game, MODE_MULTIPLAYER);
	if (isMultiplayer && game->socketConnection.type == CLIENT) {
		_notifyServerPlayerTurn(game);
		return CLIENT_PLAYED;
	}
	else if (game_selectColor(game, selectedColor) <= 0) {
		return INVALID_PLAY;
	}

	game_play_result result = END_TURN;
	if (!isMultiplayer) {
		char boardFull = _checkBoard(game);
		char allTurnsDone = (game->iTurns == MAX_TURNS);
		if (boardFull || allTurnsDone) {
			game_finish(game, !allTurnsDone);
			result = boardFull ? GAME_WON : GAME_LOST;
		}
		else {
			game->iTurns++;
		}
	}
	else {
		char hasWinner;
		_notifyCapturedPlayers(game);
		hasWinner = _hasWinner(game);
		// if only one player is remaining
		if (hasWinner) {
			_notifyWinner(game);
		}
		else {
			_notifyCurrentPlayerTurn(game, 0);
			_selectNextPlayer(game);
			_notifyCurrentPlayerTurn(game, 1);
			_broadcastGrid(game);
		}
		// the server lost
		if (game->lost == 1) {
			result = GAME_LOST;
		}
		// the host won
		else if (hasWinner) {
			result = GAME_WON;
		}
	}

	return result;
}

char game_is(s_Game *game, game_mode mode) {
	return game->mode == mode;
}

void game_setMode(s_Game *game, game_mode mode) {
	game->mode = mode;
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

int game_getGridCellColor(s_Game *game, int x, int y) {
	return game->grid[y][x].color;
}

void game_setGridCellColor(s_Game *game, int x, int y, int color) {
	game->grid[y][x].color = color;
}


char game_selectColor(s_Game* game, int color) {
	int startX, startY;
	char ret;
	startX = g_startPositionPlayers[game->currentPlayerIndex][0];
	startY = g_startPositionPlayers[game->currentPlayerIndex][1];
	ret = _spreadColor(game, color, startX, startY, 0);

	return ret;
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

void game_setGrid(s_Game* game, s_TCPpacket packet) {
	int i, j;
	for (j = 0; j < HEIGHT_GRID; ++j) {
		for (i = 0; i < WIDTH_GRID; ++i) {
			game_setGridCellColor(game, i, j, packet.data[j * WIDTH_GRID + i]);
		}
	}

	game->receivedGrid = 1;
}

/**
 * Will return 0 if the game is a client and if the server disconnected, 1
 * otherwise
 */
char game_processIncomingPackets(s_Game *game) {
	if (game->socketConnection.type == SERVER) {
		return _processServerPackets(game);;
	}
	else {
		return _processClientPackets(game);
	}
}


/** PRIVATE FUNCTIONS **/

char _checkBoard(s_Game* game) {
	signed char color = -1;
	int i, j;
	for (j = 0; j < HEIGHT_GRID; ++j) {
		for (i = 0; i < WIDTH_GRID; ++i) {
			if (color != -1 && game_getGridCellColor(game, i, j) != color) {
				return 0;
			}
			else {
				color = game_getGridCellColor(game, i, j);
			}
		}
	}

	return 1;
}

int _getGridCellOwner(s_Game *game, int x, int y) {
	return game->grid[y][x].owner;
}

void _setGridCellOwner(s_Game *game, int x, int y, int owner) {
	game->grid[y][x].owner = owner;
}

void _setPlayersInitialPosition(s_Game *game) {
	int player;
	for (player = 0; player < game->socketConnection.nbConnectedSockets + 1; ++player) {
		int startX = g_startPositionPlayers[player][0],
			startY = g_startPositionPlayers[player][1];
		_setGridCellOwner(game, startX, startY, player);
		_spreadColor(
			game,
			game_getGridCellColor(game, startX, startY),
			startX,
			startY,
			1
		);
	}
}

void _broadcastGrid(s_Game *game) {
	s_TCPpacket packet;
	packet.type = MULTIPLAYER_MESSAGE_TYPE_GRID;
	packet.size = 196;
	int nbSockets;
	// rotation matrix to turn the grid depending on the player
	// |cos(a) -sin(a)|
	// |sin(a)  cos(a)|
	int rotationMatrix[2][2];
	int shift[2];
	for (nbSockets = 0; nbSockets < game->socketConnection.nbConnectedSockets; ++nbSockets) {
		if (nbSockets == 0) {
			// matrix is rotation of 180 degrees
			rotationMatrix[0][0] = -1;
			rotationMatrix[1][0] = 0;
			rotationMatrix[0][1] = 0;
			rotationMatrix[1][1] = -1;
			shift[0] = WIDTH_GRID - 1;
			shift[1] = HEIGHT_GRID - 1;
		}
		else if (nbSockets == 1) {
			// matrix is rotation of 90 degrees
			rotationMatrix[0][0] = 0;
			rotationMatrix[1][0] = 1;
			rotationMatrix[0][1] = -1;
			rotationMatrix[1][1] = 0;
			shift[0] = 0;
			shift[1] = HEIGHT_GRID - 1;
		}
		// nbSockets == 2
		else {
			// matrix is rotation of 180 degrees
			rotationMatrix[0][0] = 0;
			rotationMatrix[1][0] = -1;
			rotationMatrix[0][1] = 1;
			rotationMatrix[1][1] = 0;
			shift[0] = WIDTH_GRID - 1;
			shift[1] = 0;
		}
		_setRotatedGridPacket(game, &packet, rotationMatrix, shift);
		multiplayer_send_message(game->socketConnection, nbSockets, packet);
	}
}

void _setRotatedGridPacket(s_Game *game, s_TCPpacket *packet, int rotationMatrix[2][2], int shift[2]) {
	int x, y, i, j;
	for (j = 0; j < HEIGHT_GRID; ++j) {
		for (i = 0; i < WIDTH_GRID; ++i) {
			// rotate point around 0 (values will then be between
			// ]-WIDTH_GRID, 0] and ]-HEIGHT_GRID, 0]
			x = i * rotationMatrix[0][0] + j * rotationMatrix[1][0];
			y = i * rotationMatrix[0][1] + j * rotationMatrix[1][1];
			// shift to go back in positives
			x += shift[0];
			y += shift[1];
			packet->data[y * WIDTH_GRID + x] = game_getGridCellColor(game, i, j);
		}
	}
}

void _notifyCurrentPlayerTurn(s_Game *game, char isTurn) {
	if (game->currentPlayerIndex == 0) {
		game->canPlay = isTurn;
		return;
	}

	s_TCPpacket packet;
	if (isTurn) {
		packet.type = MULTIPLAYER_MESSAGE_TYPE_PLAYER_TURN;
	}
	else {
		packet.type = MULTIPLAYER_MESSAGE_TYPE_PLAYER_END_TURN;
	}
	packet.size = 0;
	multiplayer_send_message(
		game->socketConnection,
		game->currentPlayerIndex - 1,
		packet
	);
}

void _selectNextPlayer(s_Game *game) {
	if (game_is(game, MODE_MULTIPLAYER)) {
		if (!game->lost) {
			game->currentPlayerIndex = (game->currentPlayerIndex + 1) % (game->socketConnection.nbConnectedSockets + 1);
		}
		else {
			game->currentPlayerIndex = (game->currentPlayerIndex + 1) % game->socketConnection.nbConnectedSockets + 1;
		}
	}
}

char _processServerPackets(s_Game *game) {
	s_TCPpacket packet;
	int indexSocketSendingMessage = -1;
	char foundMessage = multiplayer_check_clients(
		&game->socketConnection,
		&packet,
		&indexSocketSendingMessage,
		0
	);

	// the current player played and we received its choice
	if (foundMessage == MESSAGE_RECEIVED && packet.type == MULTIPLAYER_MESSAGE_TYPE_PLAYER_TURN) {
		// check message comes from good socket
		if (indexSocketSendingMessage != game->currentPlayerIndex) {
			// comes from someone else, ignore it
			return GAME_UPDATE_RESULT_CONTINUE;
		}

		game_play_result result = game_play(game, packet.data[0]);
		if (result == GAME_LOST) {
			return GAME_UPDATE_RESULT_PLAYER_LOST;
		}
		else if (result == GAME_WON) {
			return GAME_UPDATE_RESULT_PLAYER_WON;
		}
	}

	return GAME_UPDATE_RESULT_CONTINUE;
}

char _processClientPackets(s_Game *game) {
	s_TCPpacket packet;
	char state = multiplayer_check_server(&game->socketConnection, &packet);
	if (state == CONNECTION_LOST) {
		return GAME_UPDATE_RESULT_CONNECTION_LOST;
	}
	else if (state == MESSAGE_RECEIVED) {
		if (packet.type == MULTIPLAYER_MESSAGE_TYPE_GRID) {
			game_setGrid(game, packet);
			game->receivedGrid = 1;
		}
		// We received a message from the server telling us it is our turn
		// to play
		else if (packet.type == MULTIPLAYER_MESSAGE_TYPE_PLAYER_TURN) {
			game->canPlay = 1;
		}
		// The server is now telling us it is not our turn anymore
		else if (packet.type == MULTIPLAYER_MESSAGE_TYPE_PLAYER_END_TURN) {
			game->canPlay = 0;
		}
		else if (packet.type == MULTIPLAYER_MESSAGE_TYPE_PLAYER_LOST) {
			multiplayer_client_leave(&game->socketConnection);
			return GAME_UPDATE_RESULT_PLAYER_LOST;
		}
	}

	return GAME_UPDATE_RESULT_CONTINUE;
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

/**
 * Generate a random grid
 */
void _generateGrid(s_Game* game) {
	int i, j;
	time_t t;

	srand((unsigned) time(&t));
	for (j = 0; j < HEIGHT_GRID; ++j) {
		for (i = 0; i < WIDTH_GRID; ++i) {
			game_setGridCellColor(game, i, j, rand() % NB_COLORS);
			_setGridCellOwner(game, i, j, -1);
		}
	}

	game->receivedGrid = 1;
}

/**
 * Change the colors of the grid from [startX, startY] with selectedColor
 */
char _spreadColor(s_Game *game, int selectedColor, int startX, int startY, char init) {
	char toVisitFlag = 0x1,
		 visitedFlag = 0x2;
	int i, j, nbToVisit, oldColor;
	int *toVisit;
	int **visited;
	int currentOwner;

	oldColor = game_getGridCellColor(game, startX, startY);
	currentOwner = _getGridCellOwner(game, startX, startY);
	if (!init && selectedColor == oldColor) {
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
		game_setGridCellColor(game, x, y, selectedColor);
		_setGridCellOwner(game, x, y, currentOwner);

		int neighbours[4][2];
		int nbNeighbours;
		game_getNeighbours(x, y, neighbours, &nbNeighbours);
		for (i = 0; i < nbNeighbours; ++i) {
			int neighbourColor = game_getGridCellColor(
				game,
				neighbours[i][0],
				neighbours[i][1]
			);
			int neighbourOwner = _getGridCellOwner(
				game,
				neighbours[i][0],
				neighbours[i][1]
			);
			if (
				visited[neighbours[i][1]][neighbours[i][0]] == 0
				&& (
					(neighbourColor == oldColor && neighbourOwner == currentOwner)
					|| neighbourColor == selectedColor
				)
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

void _notifyServerPlayerTurn(s_Game *game) {
	s_TCPpacket packet;
	packet.type = MULTIPLAYER_MESSAGE_TYPE_PLAYER_TURN;
	packet.size = 1;
	packet.data[0] = game->iSelectedColor;
	multiplayer_send_message(game->socketConnection, -1, packet);
}

void _notifyCapturedPlayers(s_Game *game) {
	int nbCapturedPlayers = 0;
	for (int p = 0; p < 4; ++p) {
		if (p == game->currentPlayerIndex) {
			continue;
		}

		int ownerCorner = _getGridCellOwner(
			game,
			g_startPositionPlayers[p][0],
			g_startPositionPlayers[p][1]
		);
		// the current player captured this corner
		if (game->currentPlayerIndex == ownerCorner) {
			++nbCapturedPlayers;
			// the host
			if (p == 0) {
				game->lost = 1;
			}
			else {
				s_TCPpacket packet;
				packet.type = MULTIPLAYER_MESSAGE_TYPE_PLAYER_LOST;
				packet.size = 0;
				multiplayer_send_message(
					game->socketConnection,
					p - 1,
					packet
				);

				multiplayer_close_client(&game->socketConnection, p - 1);
			}
		}
	}
}

void _notifyWinner(s_Game *game) {
	if (game->currentPlayerIndex == 0) {
		return;
	}

	s_TCPpacket packet;
	packet.type = MULTIPLAYER_MESSAGE_TYPE_PLAYER_WON;
	packet.size = 0;
	multiplayer_send_message(
		game->socketConnection,
		game->currentPlayerIndex - 1,
		packet
	);
}

char _hasWinner(s_Game *game) {
	int nbClients = multiplayer_get_number_clients(game->socketConnection);
	return (game->lost && nbClients == 1) || (!game->lost && !nbClients);
}
