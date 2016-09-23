#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <time.h>
#include "globals.h"

int g_colors[NB_COLORS][3] = {
	{255, 0, 0},
	{0, 255, 0},
	{0, 0, 255},
	{255, 255, 0},
	{255, 0, 255},
	{0, 255, 255}
};

typedef struct {
	SDL_Renderer* renderer;
	SDL_Window* window;
	TTF_Font* font;
	int grid[HEIGHT_GRID][WIDTH_GRID];
	int iTurns;
	int iState;
	int iSelectedColor;
} s_Game;
SDL_Color g_White = {255, 255, 255};

int initSDL(s_Game* game, const char* title, const int x, const int y, const int w, const int h);
void handleEvents(s_Game* game, char *flags);
void generateGrid(s_Game* game);
void play(s_Game* game, char* flags);
void renderPlay(s_Game* game);
char checkBoard(s_Game* game);
void renderGrid(s_Game* game);
void render(s_Game* game, char *flags);
void renderCurrentTurn(s_Game* game);
void renderControls(s_Game* game);
void renderEndScreen(s_Game* game, const char won);
char selectColor(s_Game* game);
int popArray(int* array, int* arrayLength);
void getNeighbours(int x, int y, int neighbours[4][2], int* nbNeighbours);

int main()
{
	s_Game game;
	initSDL(&game, "Floodit", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	game.font = TTF_OpenFont("ClearSans-Medium.ttf", 18);

	// make sure SDL cleans up before exit
	atexit(SDL_Quit);

	game.iState = STATE_PLAY;

	char flags = FLAG_NEEDS_RESTART;
	while (!(flags & FLAG_DONE) && (flags & FLAG_NEEDS_RESTART) == FLAG_NEEDS_RESTART) {
		generateGrid(&game);

		// program main loop
		game.iSelectedColor = 0;
		game.iTurns = 1;
		flags ^= FLAG_NEEDS_RESTART;
		flags |= FLAG_NEEDS_REFRESH;
		while (!(flags & FLAG_DONE) && !(flags & FLAG_NEEDS_RESTART)) {
			handleEvents(&game, &flags);

			// DRAWING STARTS HERE
			if ((flags & FLAG_NEEDS_REFRESH) == FLAG_NEEDS_REFRESH) {
				render(&game, &flags);
			}
			// DRAWING ENDS HERE
		} // end main loop
	}

	// all is well ;)
	printf("Exited cleanly\n");
	return 0;
}

int initSDL(s_Game* game, const char* title, const int x, const int y, const int w, const int h) {
	char l_bReturn = 1;
	int flags;

	if (IS_GCW) {
		flags = 0;
	}
	else {
		flags = 0;
	}

	// initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL Init failed\n");
		l_bReturn = 0;
	}
	else {
		// if succeeded create our window
		game->window = SDL_CreateWindow(title, x, y, w, h, flags);
		// if the window creation succeeded create our renderer
		if (game->window == 0) {
			printf("Window creation failed\n");
			l_bReturn = 0;
		}
		else {
			game->renderer = SDL_CreateRenderer(game->window, -1, 0);
			if (game->renderer == 0) {
				printf("Renderer creation failed\n");
				l_bReturn = 0;
			}
		}
	}

	if (TTF_Init() == -1) {
		printf("TTF_Init: %s\n", TTF_GetError());
		l_bReturn = 0;
	}

	return l_bReturn;
}

void handleEvents(s_Game* game, char *flags) {
	// message processing loop
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		// check for messages
		switch (event.type) {
			// exit if the window is closed
			case SDL_QUIT:
				(*flags) |= FLAG_DONE;
				break;

			// check for keypresses
			case SDL_KEYDOWN:
				if (
					(IS_GCW && event.key.keysym.sym == SDLK_LCTRL)
					|| (!IS_GCW && event.key.keysym.sym == SDLK_SPACE)
				) {
					play(game, flags);
				}
				// exit if ESCAPE is pressed
				else if (event.key.keysym.sym == SDLK_ESCAPE) {
					(*flags) |= FLAG_DONE;
				}
				else if (game->iState == STATE_PLAY) {
					if (event.key.keysym.sym == SDLK_UP) {
						game->iSelectedColor = (game->iSelectedColor - 2 + NB_COLORS) % NB_COLORS;
						(*flags) |= FLAG_NEEDS_REFRESH;
					}
					else if (event.key.keysym.sym == SDLK_DOWN) {
						game->iSelectedColor = (game->iSelectedColor + 2) % NB_COLORS;
						(*flags) |= FLAG_NEEDS_REFRESH;
					}
					else if (event.key.keysym.sym == SDLK_LEFT) {
						game->iSelectedColor = (game->iSelectedColor - 1 + NB_COLORS) % NB_COLORS;
						(*flags) |= FLAG_NEEDS_REFRESH;
					}
					else if (event.key.keysym.sym == SDLK_RIGHT) {
						game->iSelectedColor = (game->iSelectedColor + 1) % NB_COLORS;
						(*flags) |= FLAG_NEEDS_REFRESH;
					}
				}
				break;
		}
		// end switch
	} // end of message processing
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

void play(s_Game* game, char* flags) {
	if (game->iState != STATE_PLAY) {
		game->iState = STATE_PLAY;
		(*flags) |= FLAG_NEEDS_REFRESH | FLAG_NEEDS_RESTART;
		return;
	}
	else if (selectColor(game)) {
		char finished = checkBoard(game);
		if (finished) {
			game->iState = STATE_FINISH_WON;
		}
		else if (game->iTurns == MAX_TURNS) {
			game->iState = STATE_FINISH_LOST;
		}
		else {
			game->iTurns++;
		}

		(*flags) |= FLAG_NEEDS_REFRESH;
	}
}

char checkBoard(s_Game* game) {
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

void render(s_Game* game, char *flags) {
	// Set render color to red (background will be rendered in this color)
	SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);

	// Clear window
	SDL_RenderClear(game->renderer);

	if (
		game->iState == STATE_PLAY ||
		game->iState == STATE_FINISH_WON ||
		game->iState == STATE_FINISH_LOST
	) {
		renderPlay(game);
	}
	// Render the rect to the screen
	SDL_RenderPresent(game->renderer);
	(*flags) &= ~FLAG_NEEDS_REFRESH;
}

void renderPlay(s_Game* game) {
	renderGrid(game);
	renderCurrentTurn(game);
	renderControls(game);

	if (game->iState == STATE_FINISH_WON) {
		renderEndScreen(game, 1);
	}
	else if (game->iState == STATE_FINISH_LOST) {
		renderEndScreen(game, 0);
	}
}

void renderCurrentTurn(s_Game* game) {
	char score[8];
	int textWidth, textHeight,
		textX, textY,
		widthTextSmall, widthTextLong,
		textMarginRight, textMarginBottom;

	if (IS_GCW) {
		widthTextSmall = 52;
		widthTextLong = 63;
		textMarginRight = 10;
		textMarginBottom = 30;
	}
	else {
		widthTextSmall = 52;
		widthTextLong = 63;
		textMarginRight = 10;
		textMarginBottom = 30;
	}

	snprintf(score, 8, "%d / %d", game->iTurns, MAX_TURNS);

	SDL_Surface* textSurface = TTF_RenderText_Solid(game->font, score, g_White);
	if (textSurface == NULL) {
		printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
	}
	else {
		SDL_Texture* text = SDL_CreateTextureFromSurface(game->renderer, textSurface);
		textWidth = textSurface->w;
		textHeight = textSurface->h;
		textX = SCREEN_WIDTH - textMarginRight - (game->iTurns < 10 ? widthTextSmall : widthTextLong);
		textY = SCREEN_HEIGHT - textMarginBottom;
		SDL_FreeSurface(textSurface);
		SDL_Rect renderQuad = {textX, textY, textWidth, textHeight};
		SDL_RenderCopy(game->renderer, text, NULL, &renderQuad);
		SDL_DestroyTexture(text);
	}
}

void renderGrid(s_Game* game) {
	int i, j, margin = 1;
	for (j = 0; j < HEIGHT_GRID; ++j){
		for (i = 0; i < WIDTH_GRID; ++i){
			SDL_Rect r;
			int cR, cG,cB;
			r.x = margin + i * WIDTH_GRID_PX;
			r.y = margin + j * HEIGHT_GRID_PX;
			r.w = WIDTH_GRID_PX;
			r.h = HEIGHT_GRID_PX;
			cR = g_colors[g_grid[j][i]][0];
			cG = g_colors[g_grid[j][i]][1];
			cB = g_colors[g_grid[j][i]][2];

			SDL_SetRenderDrawColor(game->renderer, cR, cG, cB, 255);
			SDL_RenderFillRect(game->renderer, &r);
		}
	}
}

void renderControls(s_Game* game) {
	int c,
		thicknessSelectedX = (SELECTED_WIDTH_CONTROL_PX - WIDTH_CONTROL_PX) / 2,
		thicknessSelectedY = (SELECTED_HEIGHT_CONTROL_PX - HEIGHT_CONTROL_PX) / 2;
	for (c = 0; c < NB_COLORS; ++c){
		SDL_Rect r;
		int cR, cG, cB;
		if (c == game->iSelectedColor) {
			// 480 + 0 +
			r.x = SCREEN_HEIGHT + CONTROL_MARGIN_X + (c % 2) * SELECTED_WIDTH_CONTROL_PX;
			r.y = CONTROL_MARGIN_Y + (c / 2) * SELECTED_HEIGHT_CONTROL_PX;
			r.w = SELECTED_WIDTH_CONTROL_PX;
			r.h = SELECTED_HEIGHT_CONTROL_PX;
		}
		else {
			r.x = SCREEN_HEIGHT + CONTROL_MARGIN_X + (c % 2) * SELECTED_WIDTH_CONTROL_PX
				+ thicknessSelectedX;
			r.y = CONTROL_MARGIN_Y + (c / 2) * SELECTED_HEIGHT_CONTROL_PX
				+ thicknessSelectedY;
			r.w = WIDTH_CONTROL_PX;
			r.h = HEIGHT_CONTROL_PX;
		}
		cR = g_colors[c][0];
		cG = g_colors[c][1];
		cB = g_colors[c][2];

		SDL_SetRenderDrawColor(game->renderer, cR, cG, cB, 255);
		SDL_RenderFillRect(game->renderer, &r);
	}
}

void renderEndScreen(s_Game* game, const char won) {
	const char *messages[2];
	int textWidth, textHeight, textX, textY, line;

	SDL_Rect bgRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

	SDL_SetRenderDrawBlendMode(game->renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 224);
	SDL_RenderFillRect(game->renderer, &bgRect);

	if (won) {
		messages[0] = "Congratulation!";
	}
	else {
		messages[0] = "You lost.";
	}

	if (IS_GCW) {
		messages[1] = "Click A to restart";
	}
	else {
		messages[1] = "Click SPACE to restart";
	}

	for (line = 0; line < 2; ++line) {
		SDL_Surface* textSurface = TTF_RenderText_Solid(game->font, messages[line], g_White);
		if (textSurface == NULL) {
			printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
		}
		else {
			SDL_Texture* text = SDL_CreateTextureFromSurface(game->renderer, textSurface);
			textWidth = textSurface->w;
			textHeight = textSurface->h;
			textX = (SCREEN_WIDTH - textWidth) / 2;
			textY = 50 + line * (textHeight + 5);
			SDL_FreeSurface(textSurface);
			SDL_Rect textRect = {textX, textY, textWidth, textHeight};
			SDL_RenderCopy(game->renderer, text, NULL, &textRect);
			SDL_DestroyTexture(text);
		}
	}
}

char selectColor(s_Game* game) {
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
		int x, y, next = popArray(toVisit, &nbToVisit);

		x = next % WIDTH_GRID;
		y = next / WIDTH_GRID;
		visited[y][x] |= visitedFlag;
		game->grid[y][x] = selectedColor;

		int neighbours[4][2];
		int nbNeighbours;
		getNeighbours(x, y, neighbours, &nbNeighbours);
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

int popArray(int* array, int* arrayLength) {
	// swap the first element with the last, reduce the length, return the old
	// first element
	if (arrayLength == 0) {
		return -1;
	}

	int elem = array[0];
	int tmp = array[(*arrayLength) - 1];
	array[(*arrayLength) - 1] = elem;
	array[0] = tmp;
	(*arrayLength) -= 1;
	return elem;
}

void getNeighbours(int x, int y, int neighbours[4][2], int* nbNeighbours) {
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
