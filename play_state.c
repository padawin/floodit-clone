#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "globals.h"
#include "game.h"
#include "play_state.h"
#include "utils.h"
#include "fsm.h"

/**
 * Game font
 */
SDL_Color g_White = {255, 255, 255};
SDL_Texture *winEndText, *loseEndText, *restartEndText, *quitEndText,
	*currentTurnText, *timerText;

#define STATE_ONGOING 1
#define STATE_FINISH_WON 2
#define STATE_FINISH_LOST 3

int g_state;

void _play(s_Game* game);
void _renderGrid(s_Game* game);
void _renderText(s_Game *game, SDL_Texture *texture, const char *text, int marginRight, int marginBottom);
void _renderTimer(s_Game* game);
void _renderCurrentTurn(s_Game* game);
void _renderControls(s_Game* game);
void _renderEndScreen(s_Game* game, const char won);

void play_state_init(s_Game *game) {
	SDL_Renderer *renderer = game->renderer;
	utils_createTextTexture(renderer, game->endFont, "Congratulations!", g_White, &winEndText);
	utils_createTextTexture(renderer, game->endFont, "You lost.", g_White, &loseEndText);
	if (IS_GCW) {
		utils_createTextTexture(renderer, game->endFont, "Press A to restart", g_White, &restartEndText);
		utils_createTextTexture(renderer, game->endFont, "Press SELECT to quit", g_White, &quitEndText);
	}
	else {
		utils_createTextTexture(renderer, game->endFont, "Press SPACE to restart", g_White, &restartEndText);
		utils_createTextTexture(renderer, game->endFont, "PRESS ESCAPE to quit", g_White, &quitEndText);
	}

	currentTurnText = 0;
	timerText = 0;
	g_state = STATE_ONGOING;
	game_start(game);
}

void play_state_clean() {
	SDL_DestroyTexture(winEndText);
	SDL_DestroyTexture(loseEndText);
	SDL_DestroyTexture(restartEndText);
	SDL_DestroyTexture(quitEndText);
	SDL_DestroyTexture(currentTurnText);
	SDL_DestroyTexture(timerText);
}

void play_state_update(s_Game *game) {
	if (!game_is(game, MODE_MULTIPLAYER)) {
		return;
	}

	if (game->socketConnection.type == CLIENT) {
		s_TCPpacket packet;
		char state = multiplayer_check_server(&game->socketConnection, &packet);
		if (state == CONNECTION_LOST) {
			fsm_setState(game, mainmenu);
		}
		else if (state == MESSAGE_RECEIVED) {
			if (packet.type == MULTIPLAYER_MESSAGE_TYPE_GRID) {
				game_setGrid(game, packet);
				game->receivedGrid = 1;
			}
			else if (packet.type == MULTIPLAYER_MESSAGE_TYPE_PLAYER_TURN) {
				game->canPlay = 1;
			}
		}
	}
}

void play_state_render(s_Game* game) {
	if (!game->receivedGrid) {
		return;
	}

	_renderGrid(game);
	_renderCurrentTurn(game);
	_renderControls(game);

	if (game_is(game, MODE_TIMED)) {
		_renderTimer(game);
	}

	if (g_state == STATE_FINISH_WON) {
		_renderEndScreen(game, 1);
	}
	else if (g_state == STATE_FINISH_LOST) {
		_renderEndScreen(game, 0);
	}
}

void _renderText(s_Game *game, SDL_Texture *texture, const char *text, int marginRight, int marginBottom) {
	int textX, textY,
		textWidth, textHeight;

	if (timerText != 0) {
		SDL_DestroyTexture(timerText);
	}

	utils_createTextTexture(game->renderer, game->scoreFont, text, g_White, &texture);
	SDL_QueryTexture(texture, NULL, NULL, &textWidth, &textHeight);
	textX = SCREEN_WIDTH - marginRight - textWidth;
	textY = SCREEN_HEIGHT - marginBottom;
	SDL_Rect rect = {textX, textY, textWidth, textHeight};
	SDL_RenderCopy(game->renderer, texture, NULL, &rect);
}

void _renderTimer(s_Game *game) {
	int textMarginRight, textMarginBottom;
	char timer[6];

	game_getTimer(game, timer);

	textMarginRight = 10;
	textMarginBottom = 50;
	_renderText(game, timerText, timer, textMarginRight, textMarginBottom);
}

void _renderCurrentTurn(s_Game* game) {
	int textMarginRight, textMarginBottom;
	char score[8];

	snprintf(score, 8, "%d / %d", game->iTurns, MAX_TURNS);

	textMarginRight = 10;
	textMarginBottom = 30;
	_renderText(game, currentTurnText, score, textMarginRight, textMarginBottom);

}

void _renderGrid(s_Game* game) {
	int i, j, margin = 1;
	for (j = 0; j < HEIGHT_GRID; ++j) {
		for (i = 0; i < WIDTH_GRID; ++i) {
			SDL_Rect r;
			int cR, cG,cB;
			r.x = margin + i * WIDTH_GRID_PX;
			r.y = margin + j * HEIGHT_GRID_PX;
			r.w = WIDTH_GRID_PX;
			r.h = HEIGHT_GRID_PX;
			cR = game->colors[game->grid[j][i]][0];
			cG = game->colors[game->grid[j][i]][1];
			cB = game->colors[game->grid[j][i]][2];

			SDL_SetRenderDrawColor(game->renderer, cR, cG, cB, 255);
			SDL_RenderFillRect(game->renderer, &r);
		}
	}
}

void _renderControls(s_Game* game) {
	int c,
		thicknessSelectedX = (SELECTED_WIDTH_CONTROL_PX - WIDTH_CONTROL_PX) / 2,
		thicknessSelectedY = (SELECTED_HEIGHT_CONTROL_PX - HEIGHT_CONTROL_PX) / 2;
	for (c = 0; c < NB_COLORS; ++c) {
		SDL_Rect r;
		int cR, cG, cB;
		if (c == game->iSelectedColor) {
			// 480 + 0 +
			r.x = SCREEN_HEIGHT + CONTROL_MARGIN_X + (c % 2) * SELECTED_WIDTH_CONTROL_PX;
			r.y = CONTROL_MARGIN_Y + (c / 2) * SELECTED_HEIGHT_CONTROL_PX;
			r.w = SELECTED_WIDTH_CONTROL_PX;
			r.h = SELECTED_HEIGHT_CONTROL_PX;
		}
		else {
			r.x = SCREEN_HEIGHT + CONTROL_MARGIN_X + (c % 2) * SELECTED_WIDTH_CONTROL_PX
				+ thicknessSelectedX;
			r.y = CONTROL_MARGIN_Y + (c / 2) * SELECTED_HEIGHT_CONTROL_PX
				+ thicknessSelectedY;
			r.w = WIDTH_CONTROL_PX;
			r.h = HEIGHT_CONTROL_PX;
		}
		cR = game->colors[c][0];
		cG = game->colors[c][1];
		cB = game->colors[c][2];

		SDL_SetRenderDrawColor(game->renderer, cR, cG, cB, 255);
		SDL_RenderFillRect(game->renderer, &r);
	}
}

/**
 * Text dimension very hacked
 */
void _renderEndScreen(s_Game* game, const char won) {
	int textWidth, textHeight, t;
	SDL_Texture *texts[3];
	SDL_Rect rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

	SDL_SetRenderDrawBlendMode(game->renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 224);
	SDL_RenderFillRect(game->renderer, &rect);

	if (won) {
		texts[0] = winEndText;
	}
	else {
		texts[0] = loseEndText;
	}
	texts[1] = restartEndText;
	texts[2] = quitEndText;

	for (t = 0; t < 3; ++t) {
		SDL_QueryTexture(texts[t], NULL, NULL, &textWidth, &textHeight);
		rect.x = (SCREEN_WIDTH - textWidth) / 2;
		rect.y = 60 + t * 40;
		rect.w = textWidth;
		rect.h = textHeight;
		SDL_RenderCopy(game->renderer, texts[t], NULL, &rect);
	}
}

void play_state_handleEvent(s_Game* game, int key) {
	if (key == SDLK_ESCAPE) {
		fsm_setState(game, mainmenu);
	}
	else if (!game->canPlay) {
		return;
	}
	else if (
		(IS_GCW && key == SDLK_LCTRL)
		|| (!IS_GCW && key == SDLK_SPACE)
	) {
		_play(game);
	}
	else if (g_state == STATE_ONGOING) {
		if (key == SDLK_UP) {
			game->iSelectedColor = (game->iSelectedColor - 2 + NB_COLORS) % NB_COLORS;
		}
		else if (key == SDLK_DOWN) {
			game->iSelectedColor = (game->iSelectedColor + 2) % NB_COLORS;
		}
		else if (key == SDLK_LEFT) {
			game->iSelectedColor = (game->iSelectedColor - 1 + NB_COLORS) % NB_COLORS;
		}
		else if (key == SDLK_RIGHT) {
			game->iSelectedColor = (game->iSelectedColor + 1) % NB_COLORS;
		}
	}
}

void _play(s_Game* game) {
	if (g_state != STATE_ONGOING) {
		game_restart(game);
		g_state = STATE_ONGOING;
		return;
	}
	else if (game_selectColor(game) > 0) {
		char finished = game_checkBoard(game);
		if (finished) {
			g_state = STATE_FINISH_WON;
			game_finish(game, 1);
		}
		else if (game->iTurns == MAX_TURNS) {
			g_state = STATE_FINISH_LOST;
			game_finish(game, 0);
		}
		else {
			game->iTurns++;
		}
	}
}
