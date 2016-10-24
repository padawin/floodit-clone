#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_net.h>
#include <stdlib.h>
#include "globals.h"
#include "game.h"
#include "fsm.h"

s_Game g_game;
s_StateMachine g_stateMachine;

int initSDL(const char* title, const int x, const int y, const int w, const int h);
void initMainMenu();
void handleEvents();
void update();
void render();
void clean();

int main() {
	const int SCREEN_FPS = 60;
	const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;

	initSDL("Floodit", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	game_init(&g_game);
	fsm_init(&g_game);

	Uint32 nextFrame;
	nextFrame = SDL_GetTicks() + SCREEN_TICKS_PER_FRAME;
	while (fsm_isRunning()) {
		handleEvents();
		update();
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

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL Init failed\n");
		l_bReturn = 0;
	}
	else {
		g_game.window = SDL_CreateWindow(title, x, y, w, h, flags);
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

void handleEvents() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				fsm_setState(&g_game, end);
				break;

			case SDL_KEYDOWN:
				fsm_handleEvent(&g_game, event.key.keysym.sym);
				break;
		}
	}
}

void update() {
	fsm_update(&g_game);
}

void render() {
	SDL_SetRenderDrawColor(g_game.renderer, 0, 0, 0, 255);
	SDL_RenderClear(g_game.renderer);

	fsm_render(&g_game);

	SDL_RenderPresent(g_game.renderer);
}

void clean() {
	game_clean(&g_game);
	TTF_Quit();
	SDLNet_Quit();
	SDL_Quit();
}
