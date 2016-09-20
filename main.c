#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH_GRID 14
#define HEIGHT_GRID 14
#define WIDTH_GRID_PX 17
#define HEIGHT_GRID_PX 17
#define WIDTH_CONTROL_PX 32
#define HEIGHT_CONTROL_PX 32
#define NB_COLORS 6

#define MAX_TURNS 25

#define FLAG_DONE 0x1
#define FLAG_NEEDS_REFRESH 0x2

#define STATE_PLAY 1
#define STATE_FINISH_LOST 3

/**
 * The game's window
 */
SDL_Window* g_window = 0;

/**
 * The game's renderer. Used to render graphics in the window
 */
SDL_Renderer* g_renderer = 0;

int g_grid[HEIGHT_GRID][WIDTH_GRID];
int g_colors[NB_COLORS][3] = {
	{255, 0, 0},
	{0, 255, 0},
	{0, 0, 255},
	{255, 255, 0},
	{255, 0, 255},
	{0, 255, 255}
};
int g_selectedColor = 0;
int g_turns = 1;
int g_state;

/**
 * Game font
 */
TTF_Font* g_Sans = 0;
SDL_Color g_White = {255, 255, 255};

int initSDL(const char* title, const int x, const int y, const int w, const int h);
void handleEvents(char *flags);
void generateGrid();
void play(char* flags);
void renderGrid();
void render(char *flags);
void renderCurrentTurn();
void renderControls();
void renderEndScreen(const char won);
char selectColor();
int popArray(int* array, int* arrayLength);
void getNeighbours(int x, int y, int neighbours[4][2], int* nbNeighbours);

int main()
{
	initSDL("Floodit", 0, 0, 320, 240);
	g_Sans = TTF_OpenFont("ClearSans-Medium.ttf", 18);

	// make sure SDL cleans up before exit
	atexit(SDL_Quit);

	g_state = STATE_PLAY;

	generateGrid();

	// program main loop
	char flags = FLAG_NEEDS_REFRESH;
	while (!(flags & FLAG_DONE)) {
		handleEvents(&flags);

		// DRAWING STARTS HERE
		if ((flags & FLAG_NEEDS_REFRESH) == FLAG_NEEDS_REFRESH) {
			render(&flags);
		}
		// DRAWING ENDS HERE
	} // end main loop

	// all is well ;)
	printf("Exited cleanly\n");
	return 0;
}

int initSDL(const char* title, const int x, const int y, const int w, const int h) {
	char l_bReturn = 1;
	int flags = SDL_WINDOW_FULLSCREEN;

	// initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL Init failed\n");
		l_bReturn = 0;
	}
	else {
		// if succeeded create our window
		g_window = SDL_CreateWindow(title, x, y, w, h, flags);
		// if the window creation succeeded create our renderer
		if (g_window == 0) {
			printf("Window creation failed\n");
			l_bReturn = 0;
		}
		else {
			g_renderer = SDL_CreateRenderer(g_window, -1, 0);
			if (g_renderer == 0) {
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

void handleEvents(char *flags) {
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
				if (event.key.keysym.sym == SDLK_LCTRL) {
					play(flags);
				}
				// exit if ESCAPE is pressed
				else if (event.key.keysym.sym == SDLK_ESCAPE) {
					(*flags) |= FLAG_DONE;
				}
				else if (g_state == STATE_PLAY) {
					if (event.key.keysym.sym == SDLK_UP) {
						g_selectedColor = (g_selectedColor - 2 + NB_COLORS) % NB_COLORS;
						(*flags) |= FLAG_NEEDS_REFRESH;
					}
					else if (event.key.keysym.sym == SDLK_DOWN) {
						g_selectedColor = (g_selectedColor + 2) % NB_COLORS;
						(*flags) |= FLAG_NEEDS_REFRESH;
					}
					else if (event.key.keysym.sym == SDLK_LEFT) {
						g_selectedColor = (g_selectedColor - 1 + NB_COLORS) % NB_COLORS;
						(*flags) |= FLAG_NEEDS_REFRESH;
					}
					else if (event.key.keysym.sym == SDLK_RIGHT) {
						g_selectedColor = (g_selectedColor + 1) % NB_COLORS;
						(*flags) |= FLAG_NEEDS_REFRESH;
					}
				}
				break;
		}
		// end switch
	} // end of message processing
}

void generateGrid() {
	int i, j;
	time_t t;

	srand((unsigned) time(&t));
	for (j = 0; j < HEIGHT_GRID; ++j){
		for (i = 0; i < WIDTH_GRID; ++i){
			g_grid[j][i] = rand() % NB_COLORS;
		}
	}
}

void play(char* flags) {
	if (selectColor()) {
		if (g_turns == MAX_TURNS) {
			g_state = STATE_FINISH_LOST;
		}
		else {
			g_turns++;
		}

		(*flags) |= FLAG_NEEDS_REFRESH;
	}
}

void render(char *flags) {
	// Set render color to red (background will be rendered in this color)
	SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);

	// Clear window
	SDL_RenderClear(g_renderer);

	if (
		g_state == STATE_PLAY ||
		g_state == STATE_FINISH_LOST
	) {
		renderGrid();
		renderCurrentTurn();
		renderControls();

		if (g_state == STATE_FINISH_LOST) {
			renderEndScreen(0);
		}
	}
	// Render the rect to the screen
	SDL_RenderPresent(g_renderer);
	(*flags) &= ~FLAG_NEEDS_REFRESH;
}

void renderCurrentTurn() {
	char score[8];
	int textWidth, textHeight, textX, textY;
	snprintf(score, 8, "%d / %d", g_turns, MAX_TURNS);

	SDL_Surface* textSurface = TTF_RenderText_Solid(g_Sans, score, g_White);
	if( textSurface == NULL ) {
		printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
	}
	else {
		SDL_Texture* text = SDL_CreateTextureFromSurface(g_renderer, textSurface);
		textWidth = textSurface->w;
		textHeight = textSurface->h;
		textX = 320 - 10 - (g_turns < 10 ? 52 : 63);
		textY = 240 - 30;
		SDL_FreeSurface(textSurface);
		SDL_Rect renderQuad = {textX, textY, textWidth, textHeight};
		SDL_RenderCopy(g_renderer, text, NULL, &renderQuad);
		SDL_DestroyTexture(text);
	}
}

void renderGrid() {
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

			SDL_SetRenderDrawColor(g_renderer, cR, cG, cB, 255);
			SDL_RenderFillRect(g_renderer, &r);
		}
	}
}

void renderControls() {
	int c;
	for (c = 0; c < NB_COLORS; ++c){
		SDL_Rect r;
		int cR, cG, cB;
		r.x = 240 + (c % 2) * 40 + 4;
		r.y = (c / 2) * 40 + 4;
		r.w = WIDTH_CONTROL_PX;
		r.h = HEIGHT_CONTROL_PX;
		cR = g_colors[c][0];
		cG = g_colors[c][1];
		cB = g_colors[c][2];

		SDL_SetRenderDrawColor(g_renderer, cR, cG, cB, 255);
		SDL_RenderFillRect(g_renderer, &r);

		if (c == g_selectedColor) {
			r.x -= 2;
			r.y -= 2;
			r.w += 4;
			r.h += 4;
			SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);
			SDL_RenderDrawRect(g_renderer, &r);
		}
	}
}

void renderEndScreen(const char won) {
	const char *messages[2];
	int textWidth, textHeight, textX, textY, line;

	SDL_Rect bgRect = {0, 0, 320, 240};

	SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 224);
	SDL_RenderFillRect(g_renderer, &bgRect);

	if (won) {
		messages[0] = "Congratulation!";
	}
	else {
		messages[0] = "You lost.";
	}

	messages[1] = "Click A to restart";

	for (line = 0; line < 2; ++line) {
		SDL_Surface* textSurface = TTF_RenderText_Solid(g_Sans, messages[line], g_White);
		if (textSurface == NULL) {
			printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
		}
		else {
			SDL_Texture* text = SDL_CreateTextureFromSurface(g_renderer, textSurface);
			textWidth = textSurface->w;
			textHeight = textSurface->h;
			textX = (320 - textWidth) / 2;
			textY = 50 + line * (textHeight + 5);
			SDL_FreeSurface(textSurface);
			SDL_Rect textRect = {textX, textY, textWidth, textHeight};
			SDL_RenderCopy(g_renderer, text, NULL, &textRect);
			SDL_DestroyTexture(text);
		}
	}
}

char selectColor() {
	int oldColor = g_grid[0][0];
	int i, j;
	if (g_selectedColor == oldColor) {
		return 0;
	}

	int nbToVisit;
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
	nbToVisit = 1;

	while (nbToVisit > 0) {
		int x, y, next = popArray(toVisit, &nbToVisit);

		x = next % WIDTH_GRID;
		y = next / WIDTH_GRID;
		visited[y][x] = 1;
		g_grid[y][x] = g_selectedColor;

		int neighbours[4][2];
		int nbNeighbours;
		getNeighbours(x, y, neighbours, &nbNeighbours);
		for (i = 0; i < nbNeighbours; ++i) {
			if (
				visited[neighbours[i][1]][neighbours[i][0]] == 0
				&& g_grid[neighbours[i][1]][neighbours[i][0]] == oldColor
			) {
				toVisit[nbToVisit++] = neighbours[i][1] * WIDTH_GRID + neighbours[i][0];
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
