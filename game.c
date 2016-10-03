#include <time.h>
#include "game.h"
#include "utils.h"
#include "high_score.h"

void generateGrid(s_Game* game);

void game_init(s_Game *game) {
	game->iState = STATE_MAIN_MENU;
	game->cFlags = FLAG_NEEDS_RESTART;
}

void game_start(s_Game *game, game_mode mode) {
	game->iState = STATE_PLAY;
	game->mode = mode;

	if (mode == MODE_TIMED) {
		game->timeStarted = SDL_GetTicks();
	}

	generateGrid(game);

	// program main loop
	game->iSelectedColor = 0;
	game->iTurns = 1;
	game_unSetFlag(game, FLAG_NEEDS_RESTART);
}

void generateGrid(s_Game* game) {
	int i, j;
	time_t t;

	srand((unsigned) time(&t));
	for (j = 0; j < HEIGHT_GRID; ++j){
		for (i = 0; i < WIDTH_GRID; ++i){
			game->grid[j][i] = rand() % NB_COLORS;
		}
	}
}

char game_checkBoard(s_Game* game) {
	signed char color = -1;
	int i, j;
	for (j = 0; j < HEIGHT_GRID; ++j){
		for (i = 0; i < WIDTH_GRID; ++i){
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

char game_selectColor(s_Game* game) {
	char toVisitFlag = 0x1,
		 visitedFlag = 0x2;
	int oldColor = game->grid[0][0];
	int i, j, nbToVisit, selectedColor;
	selectedColor = game->iSelectedColor;
	if (selectedColor == oldColor) {
		return 0;
	}

	int *toVisit;
	int **visited;

	visited = (int **) malloc(HEIGHT_GRID * sizeof(int *));
	for (i = 0; i < HEIGHT_GRID; i++) {
		visited[i] = (int *) malloc(WIDTH_GRID * sizeof(int));
	}

	toVisit = (int *) malloc(WIDTH_GRID * HEIGHT_GRID * sizeof(int *));

	for (j = 0; j < HEIGHT_GRID; ++j){
		for (i = 0; i < WIDTH_GRID; ++i){
			visited[j][i] = 0;
			toVisit[j * WIDTH_GRID + i] = 0;
		}
	}

	toVisit[0] = 0;
	visited[0][0] |= toVisitFlag;
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

char game_is(s_Game *game, char flag) {
	return (game->cFlags & flag) == flag;
}

void game_setFlag(s_Game *game, char flag) {
	game->cFlags |= flag;
}

void game_unSetFlag(s_Game *game, char flag) {
	game->cFlags &= ~flag;
}

void game_finish(s_Game *game, const char won) {
	if (game->mode == MODE_TIMED) {
		if (won) {
			high_score_save(SDL_GetTicks() - game->timeStarted, game->iTurns);
		}
	}
}
