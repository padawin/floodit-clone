#include <SDL2/SDL.h>
#include <stdlib.h>

/**
 * The game's window
 */
SDL_Window* g_window = 0;

/**
 * The game's renderer. Used to render graphics in the window
 */
SDL_Renderer* g_renderer = 0;

int initSDL(const char* title, const int x, const int y, const int w, const int h);

int main()
{
	initSDL("Floodit", 0, 0, 320, 240);

	// make sure SDL cleans up before exit
	atexit(SDL_Quit);

	// load an image
	SDL_Surface* bmp = SDL_LoadBMP("cb.bmp");
	if (!bmp) {
		printf("Unable to load bitmap: %s\n", SDL_GetError());
		return 1;
	}

	// program main loop
	int done = 0;
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

		// DRAWING ENDS HERE

		// finally, update the screen :)
		SDL_Flip(screen);
	} // end main loop

	// free loaded bitmap
	SDL_FreeSurface(bmp);

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
