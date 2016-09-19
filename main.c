#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH_GRID 14
#define HEIGHT_GRID 14
#define WIDTH_GRID_PX 17
#define HEIGHT_GRID_PX 17
#define WIDTH_CONTROL_PX 32
#define HEIGHT_CONTROL_PX 32
#define NB_COLORS 6

#define FLAG_DONE 0x1
#define FLAG_NEEDS_REFRESH 0x2

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

int initSDL(const char* title, const int x, const int y, const int w, const int h);
void handleEvents(char *flags);
void generateGrid();
void renderGrid();
void renderControls();
char selectColor();
int popArray(int* array, int* arrayLength);
void getNeighbours(int x, int y, int neighbours[4][2], int* nbNeighbours);

int main()
{
	initSDL("Floodit", 0, 0, 320, 240);

	// make sure SDL cleans up before exit
	atexit(SDL_Quit);

	generateGrid();

	// program main loop
	char flags = FLAG_NEEDS_REFRESH;
	while (!(flags & FLAG_DONE)) {
		handleEvents(&flags);

		// DRAWING STARTS HERE
		if ((flags & FLAG_NEEDS_REFRESH) == FLAG_NEEDS_REFRESH) {
			// Set render color to red (background will be rendered in this color)
			SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);

			// Clear window
			SDL_RenderClear(g_renderer);
			renderGrid();
			renderControls();
			// Render the rect to the screen
			SDL_RenderPresent(g_renderer);
			flags &= ~FLAG_NEEDS_REFRESH;
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
				// exit if ESCAPE is pressed
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					(*flags) |= FLAG_DONE;
				}
				else if (event.key.keysym.sym == SDLK_UP) {
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
				// 'A' pressed, select color
				else if (event.key.keysym.sym == SDLK_LCTRL) {
					if (selectColor()) {
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
