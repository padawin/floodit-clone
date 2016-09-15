#include <SDL2/SDL.h>
#include <stdlib.h>

#define WIDTH_GRID 14
#define HEIGHT_GRID 14
#define WIDTH_GRID_PX 17
#define HEIGHT_GRID_PX 17
#define NB_COLORS 6

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

int initSDL(const char* title, const int x, const int y, const int w, const int h);
void generateGrid();
void renderGrid();

int main()
{
	initSDL("Floodit", 0, 0, 320, 240);

	// make sure SDL cleans up before exit
	atexit(SDL_Quit);

	generateGrid();

	// program main loop
	int done = 0, needsRefresh = 1;
	while (!done) {
		// message processing loop
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			// check for messages
			switch (event.type) {
				// exit if the window is closed
				case SDL_QUIT:
					done = 1;
					break;

				// check for keypresses
				case SDL_KEYDOWN:
						// exit if ESCAPE is pressed
						if (event.key.keysym.sym == SDLK_ESCAPE)
							done = 1;
						break;
					}
				// end switch
		} // end of message processing

		// DRAWING STARTS HERE
		if (needsRefresh) {
			renderGrid();
			needsRefresh = 0;
		}
		// DRAWING ENDS HERE
	} // end main loop

	// all is well ;)
	printf("Exited cleanly\n");
	return 0;
}

int initSDL(const char* title, const int x, const int y, const int w, const int h) {
	int l_bReturn = 1;
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
	// Set render color to red (background will be rendered in this color)
	SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);

	// Clear winow
	SDL_RenderClear(g_renderer);
	int i, j, margin = 1;
	for (j = 0; j < HEIGHT_GRID; ++j){
		for (i = 0; i < WIDTH_GRID; ++i){
			// Creat a rect at pos (50, 50) that's 50 pixels wide and 50 pixels
			// high.
			SDL_Rect r;
			int cR, cG,cB;
			r.x = margin + i * WIDTH_GRID_PX;
			r.y = margin + j * HEIGHT_GRID_PX;
			r.w = WIDTH_GRID_PX;
			r.h = HEIGHT_GRID_PX;
			cR = g_colors[g_grid[j][i]][0];
			cG = g_colors[g_grid[j][i]][1];
			cB = g_colors[g_grid[j][i]][2];

			// Set render color to blue (rect will be rendered in this color)
			SDL_SetRenderDrawColor(g_renderer, cR, cG, cB, 255);

			// Render rect
			SDL_RenderFillRect(g_renderer, &r);
		}
	}

	// Render the rect to the screen
	SDL_RenderPresent(g_renderer);
}
