#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include "globals.h"
#include "game.h"
#include "menu.h"
#include "main_menu.h"
#include "play_state.h"

s_Game g_game;
s_Menu g_mainMenu;

int initSDL(const char* title, const int x, const int y, const int w, const int h);
void handleEvents();
void render();

int main() {
	initSDL("Floodit", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	g_game.scoreFont = TTF_OpenFont("ClearSans-Medium.ttf", 18);
	g_game.endFont = TTF_OpenFont("ClearSans-Medium.ttf", 18);
	g_game.menuFont = TTF_OpenFont("ClearSans-Medium.ttf", 18);
	g_game.selectedMenuFont = TTF_OpenFont("ClearSans-Medium.ttf", 24);
	g_game.colors[0][0] = 255;
	g_game.colors[0][1] = 0;
	g_game.colors[0][2] = 0;
	g_game.colors[1][0] = 0;
	g_game.colors[1][1] = 255;
	g_game.colors[1][2] = 0;
	g_game.colors[2][0] = 0;
	g_game.colors[2][1] = 0;
	g_game.colors[2][2] = 255;
	g_game.colors[3][0] = 255;
	g_game.colors[3][1] = 255;
	g_game.colors[3][2] = 0;
	g_game.colors[4][0] = 255;
	g_game.colors[4][1] = 0;
	g_game.colors[4][2] = 255;
	g_game.colors[5][0] = 0;
	g_game.colors[5][1] = 255;
	g_game.colors[5][2] = 255;

	menu_setActionsNumber(&g_mainMenu, 3);
	menu_addAction(&g_mainMenu, "Normal Mode", mainmenu_normalMode);
	menu_addAction(&g_mainMenu, "Timed Mode", mainmenu_timedMode);
	menu_addAction(&g_mainMenu, "Quit", mainmenu_quit);

	// make sure SDL cleans up before exit
	atexit(SDL_Quit);

	game_init(&g_game);
	while (!game_is(&g_game, FLAG_DONE) && game_is(&g_game, FLAG_NEEDS_RESTART)) {
		game_generateGrid(&g_game);

		// program main loop
		g_game.iSelectedColor = 0;
		g_game.iTurns = 1;
		game_unSetFlag(&g_game, FLAG_NEEDS_RESTART);
		game_setFlag(&g_game, FLAG_NEEDS_REFRESH);
		while (!game_is(&g_game, FLAG_DONE) && !game_is(&g_game, FLAG_NEEDS_RESTART)) {
			handleEvents();

			// DRAWING STARTS HERE
			if (game_is(&g_game, FLAG_NEEDS_REFRESH)) {
				render();
			}
			// DRAWING ENDS HERE
		} // end main loop
	}

	// all is well ;)
	menu_free(&g_mainMenu);
	printf("Exited cleanly\n");
	return 0;
}

int initSDL(const char* title, const int x, const int y, const int w, const int h) {
	char l_bReturn = 1;
	int flags;

	if (IS_GCW) {
		flags = SDL_WINDOW_FULLSCREEN;
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
		g_game.window = SDL_CreateWindow(title, x, y, w, h, flags);
		// if the window creation succeeded create our renderer
		if (g_game.window == 0) {
			printf("Window creation failed\n");
			l_bReturn = 0;
		}
		else {
			g_game.renderer = SDL_CreateRenderer(g_game.window, -1, 0);
			if (g_game.renderer == 0) {
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

void handleEvents() {
	// message processing loop
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		// check for messages
		switch (event.type) {
			// exit if the window is closed
			case SDL_QUIT:
				game_setFlag(&g_game, FLAG_DONE);
				break;

			// check for keypresses
			case SDL_KEYDOWN:
				switch (g_game.iState) {
					case STATE_MAIN_MENU:
						menu_handleEvent(&g_game, &g_mainMenu, event.key.keysym.sym);
						break;
					case STATE_FINISH_WON:
					case STATE_FINISH_LOST:
					case STATE_PLAY:
						play_handleEvent(&g_game, event.key.keysym.sym);
						break;
				}
				break;
		}
		// end switch
	} // end of message processing
}

void render() {
	// Set render color to red (background will be rendered in this color)
	SDL_SetRenderDrawColor(g_game.renderer, 0, 0, 0, 255);

	// Clear window
	SDL_RenderClear(g_game.renderer);

	if (g_game.iState == STATE_MAIN_MENU) {
		mainmenu_render(&g_game, &g_mainMenu);
	}
	else if (
		g_game.iState == STATE_PLAY ||
		g_game.iState == STATE_FINISH_WON ||
		g_game.iState == STATE_FINISH_LOST
	) {
		play_render(&g_game);
	}
	// Render the rect to the screen
	SDL_RenderPresent(g_game.renderer);
	game_unSetFlag(&g_game, FLAG_NEEDS_REFRESH);
}
