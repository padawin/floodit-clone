#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_net.h>
#include <stdlib.h>
#include "globals.h"
#include "game.h"
#include "menu.h"
#include "utils.h"
#include "main_menu.h"
#include "play_state.h"
#include "high_scores_state.h"

s_Game g_game;
s_Menu g_mainMenu;

int initSDL(const char* title, const int x, const int y, const int w, const int h);
void initMainMenu();
void handleEvents();
void render();
void clean();

int main() {
	const int SCREEN_FPS = 60;
	const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;

	initSDL("Floodit", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	g_game.scoreFont = TTF_OpenFont("ClearSans-Medium.ttf", 18);
	g_game.endFont = TTF_OpenFont("ClearSans-Medium.ttf", 18);
	g_game.menuFont = TTF_OpenFont("ClearSans-Medium.ttf", 18);
	g_game.selectedMenuFont = TTF_OpenFont("ClearSans-Medium.ttf", 24);
	g_game.highScoreTitleFont = TTF_OpenFont("ClearSans-Medium.ttf", 24);
	g_game.highScoreFont = TTF_OpenFont("ClearSans-Medium.ttf", 18);
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

	initMainMenu();
	game_init(&g_game);

	Uint32 nextFrame;
	nextFrame = SDL_GetTicks() + SCREEN_TICKS_PER_FRAME;
	while (!game_is(&g_game, FLAG_DONE)) {
		handleEvents();
		render();

		Uint32 now;
		now = SDL_GetTicks();
		if (nextFrame > now) {
			SDL_Delay(nextFrame - now);
		}
		nextFrame += SCREEN_TICKS_PER_FRAME;
	}

	clean();
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

	if (SDLNet_Init() == -1) {
		printf("SDLNet_Init: %s\n", SDLNet_GetError());
		l_bReturn = 0;
	}

	return l_bReturn;
}

void initMainMenu() {
	menu_setActionsNumber(&g_mainMenu, 5);
	SDL_Texture *classicModeTexture, *selectedClassicModeTextures,
		*timedModeTexture, *selectedTimedModeTexture,
		*multiplayerModeTexture, *selectedMultiplayerModeTexture,
		*highScoresTexture, *selectedHighScoresTexture,
		*quitTexture, *selectedQuitTexture;
	SDL_Color white = {255, 255, 255};
	utils_createTextTexture(g_game.renderer, g_game.menuFont, "Classic Mode", white, &classicModeTexture);
	utils_createTextTexture(g_game.renderer, g_game.selectedMenuFont, "Classic Mode", white, &selectedClassicModeTextures);
	utils_createTextTexture(g_game.renderer, g_game.menuFont, "Timed Mode", white, &timedModeTexture);
	utils_createTextTexture(g_game.renderer, g_game.selectedMenuFont, "Timed Mode", white, &selectedTimedModeTexture);
	utils_createTextTexture(g_game.renderer, g_game.menuFont, "Multiplayer Mode", white, &multiplayerModeTexture);
	utils_createTextTexture(g_game.renderer, g_game.selectedMenuFont, "Multiplayer Mode", white, &selectedMultiplayerModeTexture);
	utils_createTextTexture(g_game.renderer, g_game.menuFont, "High Scores", white, &highScoresTexture);
	utils_createTextTexture(g_game.renderer, g_game.selectedMenuFont, "High Scores", white, &selectedHighScoresTexture);
	utils_createTextTexture(g_game.renderer, g_game.menuFont, "Quit", white, &quitTexture);
	utils_createTextTexture(g_game.renderer, g_game.selectedMenuFont, "Quit", white, &selectedQuitTexture);
	menu_addAction(&g_mainMenu, mainmenu_classicMode, classicModeTexture, selectedClassicModeTextures);
	menu_addAction(&g_mainMenu, mainmenu_timedMode, timedModeTexture, selectedTimedModeTexture);
	menu_addAction(&g_mainMenu, mainmenu_multiplayerMode, multiplayerModeTexture, selectedMultiplayerModeTexture);
	menu_addAction(&g_mainMenu, mainmenu_highScores, highScoresTexture, selectedHighScoresTexture);
	menu_addAction(&g_mainMenu, mainmenu_quit, quitTexture, selectedQuitTexture);

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
					case STATE_HIGH_SCORES:
						high_scores_handleEvent(&g_game, event.key.keysym.sym);
						break;
				}
				break;
		}
		// end switch
	} // end of message processing
}

void render() {
	SDL_SetRenderDrawColor(g_game.renderer, 0, 0, 0, 255);
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
	else if (g_game.iState == STATE_HIGH_SCORES) {
		high_scores_render(&g_game);
	}

	// Render the rect to the screen
	SDL_RenderPresent(g_game.renderer);
}

void clean() {
	TTF_CloseFont(g_game.scoreFont);
	g_game.scoreFont = NULL;
	TTF_CloseFont(g_game.endFont);
	g_game.endFont = NULL;
	TTF_CloseFont(g_game.menuFont);
	g_game.menuFont = NULL;
	TTF_CloseFont(g_game.selectedMenuFont);
	g_game.selectedMenuFont = NULL;
	TTF_CloseFont(g_game.highScoreFont);
	g_game.highScoreFont = NULL;
	TTF_CloseFont(g_game.highScoreTitleFont);
	g_game.highScoreTitleFont = NULL;
	menu_free(&g_mainMenu);
	TTF_Quit();
	SDLNet_Quit();
	SDL_Quit();
}
