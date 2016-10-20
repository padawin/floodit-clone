#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <ifaddrs.h>
#include "globals.h"
#include "game.h"
#include "menu.h"
#include "IPConfigurator.h"
#include "multiplayer.h"
#include "multiplayer_setup_state.h"
#include "fsm.h"
#include "utils.h"
#include "net.h"

s_Menu g_hostJoinMenu;
SDL_Color white = {255, 255, 255};
SDL_Texture *selectPlayersTexture;
SDL_Texture *serverIPTexture;
SDL_Texture *IPTexture;
SDL_Texture *selectNumberTexture;
SDL_Texture *hostIpTexture;
SDL_Texture *ipsTextures[5];
int g_nbIps;
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
struct ifaddrs *g_ifap;

void _initMenus(s_Game *game);
void _initIPs(s_Game *game);
void _hostGameAction(s_Game *game);
void _joinGameAction(s_Game *game);
void _backAction(s_Game *game);
void _handleIPSelectionEvent(s_Game *game, int key);
char _addDigitToIP(s_Game *game);
void _removeDigitFromIP(s_Game *game);
void _createIPTexture(s_Game *game);

void multiplayer_setup_state_init(s_Game *game) {
	_initMenus(game);
	_initIPs(game);
	g_localState = STATE_HOST_JOIN;
}

void _initIPs(s_Game *game) {
	net_getIPs(&g_ifap);
	g_nbIps = 0;
	char *interface, *address;
	while (
		g_nbIps < 5
		&& net_getNextIP(&g_ifap, &interface, &address)
	) {
		char text[25];
		snprintf(text, 25, "%s (%s)", address, interface);
		utils_createTextTexture(
			game->renderer,
			game->menuFont,
			text,
			white,
			&ipsTextures[g_nbIps]
		);
		++g_nbIps;
	}

	utils_createTextTexture(
		game->renderer,
		game->menuFont,
		g_nbIps > 1 ? "Host possible IPs:" : "Host IP:",
		white,
		&hostIpTexture
	);
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

void multiplayer_setup_state_clean() {
	menu_free(&g_hostJoinMenu);
	SDL_DestroyTexture(selectNumberTexture);
	SDL_DestroyTexture(selectPlayersTexture);
	SDL_DestroyTexture(serverIPTexture);
	SDL_DestroyTexture(IPTexture);
	SDL_DestroyTexture(hostIpTexture);
	net_freeIfAddr(g_ifap);
	while (g_nbIps--) {
		SDL_DestroyTexture(ipsTextures[g_nbIps]);
	}
}

void multiplayer_setup_state_update(s_Game* game) {
	if (g_localState == STATE_WAIT_FOR_CLIENTS) {
		multiplayer_check_connections(&game->socketConnection);
	}
}

void multiplayer_setup_state_render(s_Game* game) {
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
	else if (g_localState == STATE_WAIT_FOR_CLIENTS) {
		SDL_QueryTexture(hostIpTexture, NULL, NULL, &textWidth, &textHeight);
		SDL_Rect rect = {50, 30, textWidth, textHeight};
		SDL_RenderCopy(game->renderer, hostIpTexture, NULL, &rect);

		int i, textWidth, textHeight;
		for (i = 0; i < g_nbIps; ++i) {
			SDL_QueryTexture(ipsTextures[i], NULL, NULL, &textWidth, &textHeight);
			SDL_Rect rect = {50, 55 + 24 * i, textWidth, textHeight};
			SDL_RenderCopy(game->renderer, ipsTextures[i], NULL, &rect);
		}
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

void multiplayer_setup_state_handleEvent(s_Game* game, int key) {

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
			game_setFlag(game, FLAG_MULTIPLAYER);
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
	fsm_setState(game, mainmenu);
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
	else if ((IS_GCW && key == SDLK_LSHIFT) || (!IS_GCW && key == SDLK_BACKSPACE)) {
		_removeDigitFromIP(game);
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

void _removeDigitFromIP(s_Game *game) {
	IPConfigurator_removeChar(&g_IPConfigurator);
	_createIPTexture(game);
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
