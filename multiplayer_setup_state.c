#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "globals.h"
#include "game.h"
#include "menu.h"
#include "multiplayer_setup_state.h"
#include "utils.h"

s_Menu g_hostJoinMenu;

int STATE_HOST_JOIN = 1;
int STATE_HOST_SETUP = 2;
int g_localState;

void _initMenus();
void _hostGameAction(s_Game *game);
void _joinGameAction(s_Game *game);
void _backAction(s_Game *game);
void _renderHostJoinMenu();

void multiplayer_setup_state_init(s_Game *game) {
	_initMenus(game);
	g_localState = STATE_HOST_JOIN;
}

void _initMenus(s_Game *game) {
	menu_setActionsNumber(&g_hostJoinMenu, 3);
	SDL_Texture *hostGameTexture, *selectedHostGameTexture,
		*joinGameTexture, *selectedJoinGameTexture,
		*backTexture, *selectedBackTexture;
	SDL_Color white = {255, 255, 255};
	utils_createTextTexture(game->renderer, game->menuFont, "Host game", white, &hostGameTexture);
	utils_createTextTexture(game->renderer, game->selectedMenuFont, "Host game", white, &selectedHostGameTexture);
	utils_createTextTexture(game->renderer, game->menuFont, "Join game", white, &joinGameTexture);
	utils_createTextTexture(game->renderer, game->selectedMenuFont, "Join game", white, &selectedJoinGameTexture);
	utils_createTextTexture(game->renderer, game->menuFont, "Back", white, &backTexture);
	utils_createTextTexture(game->renderer, game->selectedMenuFont, "Back", white, &selectedBackTexture);
	menu_addAction(&g_hostJoinMenu, _hostGameAction, hostGameTexture, selectedHostGameTexture);
	menu_addAction(&g_hostJoinMenu, _joinGameAction, joinGameTexture, selectedJoinGameTexture);
	menu_addAction(&g_hostJoinMenu, _backAction, backTexture, selectedBackTexture);
}

void multiplayer_setup_state_clean(s_Game *game) {
	menu_free(&g_hostJoinMenu);
}

void multiplayer_setup_render(s_Game* game) {
	if (g_localState == STATE_HOST_JOIN) {
		menu_render(game, &g_hostJoinMenu);
	}
}

void multiplayer_setup_handleEvent(s_Game* game, int key) {
	if (g_localState == STATE_HOST_JOIN) {
		menu_handleEvent(game, &g_hostJoinMenu, key);
	}
	else if (g_localState == STATE_HOST_SETUP) {
		if (
			(IS_GCW && key == SDLK_LCTRL)
			|| (!IS_GCW && key == SDLK_SPACE)
		) {
			printf("Host game!\n");
		}
		else if (key == SDLK_ESCAPE) {
			g_localState = STATE_HOST_JOIN;
		}
		else if (key == SDLK_UP) {
			printf("Increase players number\n");
		}
		else if (key == SDLK_DOWN) {
			printf("Decrease players number\n");
		}
	}
}

void _hostGameAction(s_Game *game) {
	g_localState = STATE_HOST_SETUP;
}

void _joinGameAction(s_Game *game) {
	printf("Join Game \n");
}

void _backAction(s_Game *game) {
	game_init(game);
}

void _renderHostJoinMenu() {

}
