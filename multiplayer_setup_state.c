#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "globals.h"
#include "game.h"
#include "menu.h"
#include "IPConfigurator.h"
#include "multiplayer.h"
#include "multiplayer_setup_state.h"
#include "utils.h"

s_Menu g_hostJoinMenu;
SDL_Color white = {255, 255, 255};
SDL_Texture *selectPlayersTexture;
SDL_Texture *serverIPTexture;
SDL_Texture *IPTexture;
SDL_Texture *selectNumberTexture;
int g_playersNumber = 2;
int g_IPKeyboardSelectedValue = 0;
uint8_t g_ipCharMapping[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0, '.'};
int g_keypadWidth = 3,
	g_keypadHeight = 4;
s_IpAddressConfigurator g_IPConfigurator;

int STATE_HOST_JOIN = 1;
int STATE_HOST_SETUP = 2;
int STATE_WAIT_FOR_CLIENTS = 3;
int STATE_JOIN_SETUP = 4;
int g_localState;

void _initMenus();
void _hostGameAction(s_Game *game);
void _joinGameAction(s_Game *game);
void _backAction(s_Game *game);
void _handleIPSelectionEvent(s_Game *game, int key);
char _addDigitToIP(s_Game *game);
void _createIPTexture(s_Game *game);

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

	utils_createTextTexture(
		game->renderer,
		game->menuFont,
		"Players:",
		white,
		&selectPlayersTexture
	);

	utils_createTextTexture(
		game->renderer,
		game->menuFont,
		"Server IP:",
		white,
		&serverIPTexture
	);
	utils_loadImageTexture(game->renderer, "resources/text-atlas.png", &selectNumberTexture);
}

void multiplayer_setup_state_clean(s_Game *game) {
	menu_free(&g_hostJoinMenu);
	SDL_DestroyTexture(selectNumberTexture);
	SDL_DestroyTexture(selectPlayersTexture);
	SDL_DestroyTexture(serverIPTexture);
	SDL_DestroyTexture(IPTexture);
}

void multiplayer_setup_update(s_Game* game) {
	if (g_localState == STATE_WAIT_FOR_CLIENTS) {
		multiplayer_check_connections(&game->socketConnection);
	}
}

void multiplayer_setup_render(s_Game* game) {
	int textWidth, textHeight;
	if (g_localState == STATE_HOST_JOIN) {
		menu_render(game, &g_hostJoinMenu);
	}
	else if (g_localState == STATE_HOST_SETUP) {
		SDL_QueryTexture(selectPlayersTexture, NULL, NULL, &textWidth, &textHeight);
		SDL_Rect rect = {50, 30, textWidth, textHeight};
		SDL_RenderCopy(game->renderer, selectPlayersTexture, NULL, &rect);

		SDL_Rect srcRect = {11 * (g_playersNumber - 2), 0, 11, 30};
		SDL_Rect destRect = {55 + textWidth, 30, 11, 30};
		SDL_RenderCopyEx(
			game->renderer,
			selectNumberTexture,
			&srcRect, &destRect,
			0, 0, 0
		);
	}
	else if (g_localState == STATE_JOIN_SETUP) {
		SDL_QueryTexture(serverIPTexture, NULL, NULL, &textWidth, &textHeight);
		SDL_Rect rect = {50, 30, textWidth, textHeight};
		SDL_RenderCopy(game->renderer, serverIPTexture, NULL, &rect);

		SDL_QueryTexture(IPTexture, NULL, NULL, &textWidth, &textHeight);
		SDL_Rect IPRect = {50, 60, textWidth, textHeight};
		SDL_RenderCopy(game->renderer, IPTexture, NULL, &IPRect);

		if (IS_GCW) {
			SDL_Rect srcRect = {0, 30, 69, 120};
			SDL_Rect destRect = {(SCREEN_WIDTH - 69) / 2, 100, 69, 120};
			SDL_RenderCopyEx(
				game->renderer,
				selectNumberTexture,
				&srcRect, &destRect,
				0, 0, 0
			);

			SDL_Rect srcSelectRect = {46, 0, 23, 30};
			SDL_Rect destSelectRect = {
				(SCREEN_WIDTH - 69) / 2 + 23 * (g_IPKeyboardSelectedValue % g_keypadWidth),
				100 + 30 * (g_IPKeyboardSelectedValue / g_keypadWidth),
				23, 30
			};
			SDL_RenderCopyEx(
				game->renderer,
				selectNumberTexture,
				&srcSelectRect, &destSelectRect,
				0, 0, 0
			);
		}
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
			multiplayer_create_connection(&game->socketConnection, 0);
			g_localState = STATE_WAIT_FOR_CLIENTS;
		}
		else if (key == SDLK_ESCAPE) {
			g_localState = STATE_HOST_JOIN;
		}
		else if (key == SDLK_UP && g_playersNumber < MULTIPLAYER_MAX_PLAYERS_NUMBER) {
			++g_playersNumber;
		}
		else if (key == SDLK_DOWN && g_playersNumber > MULTIPLAYER_MIN_PLAYERS_NUMBER) {
			--g_playersNumber;
		}
	}
	else if (g_localState == STATE_JOIN_SETUP) {
		if (key == SDLK_ESCAPE) {
			g_localState = STATE_HOST_JOIN;
		}
		else {
			_handleIPSelectionEvent(game, key);
		}
	}
}

void _hostGameAction(s_Game *game) {
	g_localState = STATE_HOST_SETUP;
}

void _joinGameAction(s_Game *game) {
	g_localState = STATE_JOIN_SETUP;
	g_IPConfigurator = IPConfigurator_create();
	_createIPTexture(game);
}

void _backAction(s_Game *game) {
	multiplayer_setup_state_clean(game);
	game_init(game);
}

void _handleIPSelectionEvent(s_Game *game, int key) {
	int x = g_IPKeyboardSelectedValue % g_keypadWidth,
		y = g_IPKeyboardSelectedValue / g_keypadWidth;
	if ((IS_GCW && key == SDLK_LCTRL) || (!IS_GCW && key == SDLK_SPACE)) {
		if (_addDigitToIP(game)) {
			char ip[16];
			IPConfigurator_toString(&g_IPConfigurator, ip, 1);
			multiplayer_create_connection(&game->socketConnection, ip);
		}
		return;
	}
	else if (key == SDLK_RIGHT) {
		x = (x + 1) % g_keypadWidth;
	}
	else if (key == SDLK_LEFT) {
		x = (g_keypadWidth + x - 1) % g_keypadWidth;
	}
	else if (key == SDLK_UP) {
		y = (g_keypadHeight + y - 1) % g_keypadHeight;
	}
	else if (key == SDLK_DOWN) {
		y = (y + 1) % g_keypadHeight;
	}

	g_IPKeyboardSelectedValue = (y * g_keypadWidth + x);
}

char _addDigitToIP(s_Game *game) {
	if (g_IPKeyboardSelectedValue == 11) {
		return 1;
	}

	IPConfigurator_addChar(
		&g_IPConfigurator,
		g_ipCharMapping[g_IPKeyboardSelectedValue]
	);

	_createIPTexture(game);
	return 0;
}

void _createIPTexture(s_Game *game) {
	if (IPTexture != 0) {
		SDL_DestroyTexture(IPTexture);
	}

	char ip[16];
	IPConfigurator_toString(&g_IPConfigurator, ip, 0);
	utils_createTextTexture(
		game->renderer,
		game->menuFont,
		ip,
		white,
		&IPTexture
	);
}
