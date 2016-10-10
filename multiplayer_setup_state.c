#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "globals.h"
#include "game.h"
#include "menu.h"
#include "multiplayer.h"
#include "multiplayer_setup_state.h"
#include "utils.h"

s_Menu g_hostJoinMenu;
SDL_Color white = {255, 255, 255};
SDL_Texture *selectPlayersNumberTexture;
int g_playersNumber = 2;

int STATE_HOST_JOIN = 1;
int STATE_HOST_SETUP = 2;
int STATE_WAIT_FOR_CLIENTS = 3;
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
	SDL_DestroyTexture(selectPlayersNumberTexture);
}

void multiplayer_setup_render(s_Game* game) {
	if (g_localState == STATE_HOST_JOIN) {
		menu_render(game, &g_hostJoinMenu);
	}
	else if (g_localState == STATE_HOST_SETUP) {
		if (selectPlayersNumberTexture != 0) {
			SDL_DestroyTexture(selectPlayersNumberTexture);
		}
		char txt[19];
		int textWidth, textHeight;
		snprintf(txt, 19, "Players: < %d >", g_playersNumber);
		utils_createTextTexture(game->renderer, game->menuFont, txt, white, &selectPlayersNumberTexture);
		SDL_QueryTexture(selectPlayersNumberTexture, NULL, NULL, &textWidth, &textHeight);
		SDL_Rect rect = {50, 30, textWidth, textHeight};
		SDL_RenderCopy(game->renderer, selectPlayersNumberTexture, NULL, &rect);
	}
}

void multiplayer_setup_handleEvent(s_Game* game, int key) {

	if (g_localState == STATE_HOST_JOIN) {
		if (key == SDLK_ESCAPE) {
			_backAction(game);
		}
		else {
			menu_handleEvent(game, &g_hostJoinMenu, key);
		}
	}
	else if (g_localState == STATE_HOST_SETUP) {
		if (
			(IS_GCW && key == SDLK_LCTRL)
			|| (!IS_GCW && key == SDLK_SPACE)
		) {
			multiplayer_create_server(&game->socketConnection);
			g_localState = STATE_WAIT_FOR_CLIENTS;
		}
		else if (key == SDLK_ESCAPE) {
			g_localState = STATE_HOST_JOIN;
		}
		else if (key == SDLK_RIGHT) {
			g_playersNumber = (g_playersNumber - 2 + 1) % 3 + 2;
		}
		else if (key == SDLK_LEFT) {
			g_playersNumber = (g_playersNumber - 2 - 1) % 3 + 2;
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
	multiplayer_setup_state_clean(game);
	game_init(game);
}

void _renderHostJoinMenu() {

}
