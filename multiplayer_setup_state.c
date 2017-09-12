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
SDL_Color white = {255, 255, 255, 255};
SDL_Texture *selectPlayersTexture;
SDL_Texture *serverIPTexture;
SDL_Texture *IPTexture;
SDL_Texture *selectNumberTexture;
SDL_Texture *hostIpTexture;
SDL_Texture *ipsTextures[5];
SDL_Texture *connectedClientsTexture;
SDL_Texture *waitForGameTexture;
SDL_Texture *noConnectionTexture;
SDL_Texture *attemptConnectTexture;
SDL_Texture *serverFullTexture;
SDL_Texture *errorContactServerTexture;
SDL_Texture **currentError = NULL;
int g_nbIps;
int g_playersNumber = 2;
int g_IPKeyboardSelectedValue = 0;
char g_ipCharMapping[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0, '.'};
int g_keypadWidth = 3,
	g_keypadHeight = 4;

unsigned int g_pongTimeout = 3000,
	g_timestampWaitForServer = 0,
	g_timestampLastPing = 0,
	g_pingTimeout = 500;

s_IpAddressConfigurator g_IPConfigurator;

int STATE_HOST_JOIN = 1;
int STATE_HOST_SETUP = 2;
int STATE_WAIT_FOR_CLIENTS = 3;
int STATE_JOIN_SETUP = 4;
int STATE_JOIN_SETUP_WAIT_PONG = 5;
int STATE_WAIT_FOR_GAME = 6;
int g_localState;
struct ifaddrs *g_ifap;

void _initMenus(s_Game *game);
void _initIPs(s_Game *game);
void _hostGameAction();
void _joinGameAction(s_Game *game);
void _backAction(s_Game *game);
void _handleIPSelectionEventGCW(s_Game *game, int key);
void _handleIPSelectionEvent(s_Game *game, int key);
void _addDigitToIP(s_Game *game, char digit);
void _removeDigitFromIP(s_Game *game);
void _createIPTexture(s_Game *game);
void _setSetupError(SDL_Texture **error);
void _connectToHost(s_Game *game);
void _setPingState(s_Game *game);
void _multiplayerRenderText(s_Game *game, SDL_Texture *text, int x, int y, int *textWidthOut);

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
	utils_createTextTexture(game->renderer, game->menuFont, "Wait for game to start...", white, &waitForGameTexture);
	utils_createTextTexture(game->renderer, game->menuFont, "Host game", white, &hostGameTexture);
	utils_createTextTexture(game->renderer, game->selectedMenuFont, "Host game", white, &selectedHostGameTexture);
	utils_createTextTexture(game->renderer, game->menuFont, "Join game", white, &joinGameTexture);
	utils_createTextTexture(game->renderer, game->selectedMenuFont, "Join game", white, &selectedJoinGameTexture);
	utils_createTextTexture(game->renderer, game->menuFont, "Back", white, &backTexture);
	utils_createTextTexture(game->renderer, game->selectedMenuFont, "Back", white, &selectedBackTexture);
	utils_createTextTexture(game->renderer, game->menuFont, "Players:", white, &selectPlayersTexture);
	utils_createTextTexture(game->renderer, game->menuFont, "Server IP:", white, &serverIPTexture);
	utils_createTextTexture(game->renderer, game->menuFont, "Unable to create connection", white, &noConnectionTexture);
	utils_createTextTexture(game->renderer, game->menuFont, "Unable to contact server", white, &errorContactServerTexture);
	utils_createTextTexture(game->renderer, game->menuFont, "Trying to connect...", white, &attemptConnectTexture);
	utils_createTextTexture(game->renderer, game->menuFont, "Server full", white, &serverFullTexture);
	menu_addAction(&g_hostJoinMenu, _hostGameAction, hostGameTexture, selectedHostGameTexture);
	menu_addAction(&g_hostJoinMenu, _joinGameAction, joinGameTexture, selectedJoinGameTexture);
	menu_addAction(&g_hostJoinMenu, _backAction, backTexture, selectedBackTexture);

	utils_loadImageTexture(game->renderer, "resources/text-atlas.png", &selectNumberTexture);
}

void multiplayer_setup_state_clean() {
	menu_free(&g_hostJoinMenu);
	SDL_DestroyTexture(selectNumberTexture);
	SDL_DestroyTexture(selectPlayersTexture);
	SDL_DestroyTexture(serverIPTexture);
	SDL_DestroyTexture(IPTexture);
	SDL_DestroyTexture(hostIpTexture);
	SDL_DestroyTexture(connectedClientsTexture);
	SDL_DestroyTexture(waitForGameTexture);
	SDL_DestroyTexture(noConnectionTexture);
	SDL_DestroyTexture(attemptConnectTexture);
	SDL_DestroyTexture(serverFullTexture);
	SDL_DestroyTexture(errorContactServerTexture);
	net_freeIfAddr(g_ifap);
	while (g_nbIps--) {
		SDL_DestroyTexture(ipsTextures[g_nbIps]);
	}
}

void multiplayer_setup_state_update(s_Game* game) {
	if (g_localState == STATE_WAIT_FOR_CLIENTS) {
		multiplayer_accept_client(&game->socketConnection);
		multiplayer_check_clients(&game->socketConnection, 0, 0, 1);

		if (multiplayer_is_room_full(&game->socketConnection)) {
			s_TCPpacket packet;
			packet.type = MULTIPLAYER_MESSAGE_TYPE_GAME_START;
			packet.size = 0;
			multiplayer_broadcast(&game->socketConnection, packet);
			fsm_setState(game, play);
		}
	}
	else if (g_localState == STATE_JOIN_SETUP_WAIT_PONG) {
		// multiplayer_interrogate_server
		// printf("check if server answered\n");
		unsigned int ticks = SDL_GetTicks();
		if (ticks - g_timestampWaitForServer > g_pongTimeout) {
			_setSetupError(&noConnectionTexture);
		}
		else if (ticks - g_timestampLastPing > g_pingTimeout) {
			g_timestampLastPing = ticks;
			if (multiplayer_check_server_pong(&game->socketConnection)) {
				_connectToHost(game);
			}
		}
	}
	else if (g_localState == STATE_WAIT_FOR_GAME) {
		s_TCPpacket packet;
		char state = multiplayer_check_server(&game->socketConnection, &packet);
		if (state == CONNECTION_LOST) {
			multiplayer_clean(&game->socketConnection);
			g_localState = STATE_HOST_JOIN;
		}
		else if (state == MESSAGE_RECEIVED) {
			if (packet.type == MULTIPLAYER_MESSAGE_TYPE_GAME_START) {
				fsm_setState(game, play);
			}
			else if (packet.type == MULTIPLAYER_MESSAGE_TYPE_SERVER_FULL) {
				multiplayer_clean(&game->socketConnection);
				g_localState = STATE_JOIN_SETUP;
				_setSetupError(&serverFullTexture);
			}
		}
	}
}

void multiplayer_setup_state_render(s_Game* game) {
	int textWidth;
	if (g_localState == STATE_HOST_JOIN) {
		menu_render(game, &g_hostJoinMenu);
	}
	else if (g_localState == STATE_HOST_SETUP) {
		_multiplayerRenderText(game, selectPlayersTexture, 50, 30, &textWidth);

		SDL_Rect srcRect = {11 * (g_playersNumber - 2), 0, 11, 30};
		SDL_Rect destRect = {55 + textWidth, 30, 11, 30};
		SDL_RenderCopyEx(
			game->renderer,
			selectNumberTexture,
			&srcRect, &destRect,
			0, 0, 0
		);

		if (currentError != NULL) {
			_multiplayerRenderText(game, *currentError, 50, 60, NULL);
		}
	}
	else if (g_localState == STATE_WAIT_FOR_CLIENTS) {
		_multiplayerRenderText(game, hostIpTexture, 50, 30, NULL);

		int i;
		for (i = 0; i < g_nbIps; ++i) {
			_multiplayerRenderText(game, ipsTextures[i], 50, 55 + 24 * i, NULL);
		}

		char connectedClientsText[25];
		SDL_DestroyTexture(connectedClientsTexture);
		snprintf(
			connectedClientsText,
			25,
			"Connected clients: %d / %d",
			game->socketConnection.nbConnectedSockets,
			game->socketConnection.nbMaxSockets
		);
		utils_createTextTexture(
			game->renderer,
			game->menuFont,
			connectedClientsText,
			white,
			&connectedClientsTexture
		);
		_multiplayerRenderText(game, connectedClientsTexture, 50, 55 + 24 * g_nbIps + 14, NULL);
	}
	else if (g_localState == STATE_JOIN_SETUP || g_localState == STATE_JOIN_SETUP_WAIT_PONG) {
		_multiplayerRenderText(game, serverIPTexture, 50, 30, NULL);
		_multiplayerRenderText(game, IPTexture, 50, 60, NULL);

		if (IS_GCW) {
			SDL_Rect srcRect = {0, 30, 69, 120};
			SDL_Rect destRect = {(SCREEN_WIDTH - 69) / 2, 85, 69, 120};
			SDL_RenderCopyEx(
				game->renderer,
				selectNumberTexture,
				&srcRect, &destRect,
				0, 0, 0
			);

			SDL_Rect srcSelectRect = {46, 0, 23, 30};
			SDL_Rect destSelectRect = {
				(SCREEN_WIDTH - 69) / 2 + 23 * (g_IPKeyboardSelectedValue % g_keypadWidth),
				85 + 30 * (g_IPKeyboardSelectedValue / g_keypadWidth),
				23, 30
			};
			SDL_RenderCopyEx(
				game->renderer,
				selectNumberTexture,
				&srcSelectRect, &destSelectRect,
				0, 0, 0
			);
		}

		if (currentError != NULL) {
			_multiplayerRenderText(game, *currentError, 50, 210, NULL);
		}
	}
	else if (g_localState == STATE_WAIT_FOR_GAME) {
		_multiplayerRenderText(game, waitForGameTexture, 50, 30, NULL);
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
			if (!multiplayer_create_connection(&game->socketConnection, 0, TCP)) {
				_setSetupError(&noConnectionTexture);
			}
			else {
				game_setMode(game, MODE_MULTIPLAYER);
				multiplayer_initHost(&game->socketConnection, g_playersNumber);
				g_localState = STATE_WAIT_FOR_CLIENTS;
			}
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
	else if (g_localState == STATE_WAIT_FOR_CLIENTS) {
		if (key == SDLK_ESCAPE) {
			g_localState = STATE_HOST_SETUP;
			multiplayer_clean(&game->socketConnection);
			game_setMode(game, MODE_CLASSIC);
		}
	}
	else if (g_localState == STATE_JOIN_SETUP) {
		if (key == SDLK_ESCAPE) {
			g_localState = STATE_HOST_JOIN;
			currentError = NULL;
		}
		else if (IS_GCW) {
			_handleIPSelectionEventGCW(game, key);
		}
		else {
			_handleIPSelectionEvent(game, key);
		}
	}
	else if (g_localState == STATE_JOIN_SETUP_WAIT_PONG) {
		if (key == SDLK_ESCAPE) {
			g_localState = STATE_JOIN_SETUP;
			multiplayer_clean(&game->socketConnection);
			currentError = NULL;
		}
	}
}

void _setSetupError(SDL_Texture **error) {
	currentError = error;
}

void _multiplayerRenderText(s_Game *game, SDL_Texture *text, int x, int y, int *textWidthOut) {
	int textWidth = 0,
		textHeight = 0;
	SDL_QueryTexture(text, NULL, NULL, &textWidth, &textHeight);
	SDL_Rect rect = {x, y, textWidth, textHeight};
	SDL_RenderCopy(game->renderer, text, NULL, &rect);
	if (textWidthOut != NULL) {
		*textWidthOut = textWidth;
	}
}

void _hostGameAction() {
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
	if ((SDLK_0 <= key && key <= SDLK_9) || key == SDLK_PERIOD) {
		if (key != SDLK_PERIOD) {
			key = key - '0';
		}
		_addDigitToIP(game, (char) key);
	}
	else if (key == SDLK_SPACE && g_IPConfigurator.ipAddress > 0) {
		_setPingState(game);
	}
	else if (key == SDLK_BACKSPACE) {
		_removeDigitFromIP(game);
	}
}

void _handleIPSelectionEventGCW(s_Game *game, int key) {
	int x = g_IPKeyboardSelectedValue % g_keypadWidth,
		y = g_IPKeyboardSelectedValue / g_keypadWidth;
	if (key == SDLK_LCTRL) {
		if (g_IPKeyboardSelectedValue != 11) {
			_addDigitToIP(game, g_ipCharMapping[g_IPKeyboardSelectedValue]);
		}
		else if (g_IPConfigurator.ipAddress > 0) {
			_setPingState(game);
		}
		return;
	}
	else if (key == SDLK_LSHIFT) {
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

void _setPingState(s_Game *game) {
	g_localState = STATE_JOIN_SETUP_WAIT_PONG;
	g_timestampWaitForServer = SDL_GetTicks();
	char ip[16];
	IPConfigurator_toString(&g_IPConfigurator, ip, 1);
	if (!multiplayer_create_connection(&game->socketConnection, ip, PING)) {
		_setSetupError(&errorContactServerTexture);
	}
	else {
		_setSetupError(&attemptConnectTexture);
	}
}

void _connectToHost(s_Game *game) {
	char ip[16];
	IPConfigurator_toString(&g_IPConfigurator, ip, 1);
	if (!multiplayer_create_connection(&game->socketConnection, ip, TCP)) {
		_setSetupError(&noConnectionTexture);
	}
	else {
		multiplayer_initClient(&game->socketConnection);
		g_localState = STATE_WAIT_FOR_GAME;
		game_setMode(game, MODE_MULTIPLAYER);
	}
}

void _addDigitToIP(s_Game *game, char digit) {
	IPConfigurator_addChar(&g_IPConfigurator, digit);
	_createIPTexture(game);
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
