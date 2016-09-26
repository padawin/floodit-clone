#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "globals.h"
#include "game.h"
#include "play_state.h"

/**
 * Game font
 */
SDL_Color g_White = {255, 255, 255};

void play(s_Game* game);
void renderGrid(s_Game* game);
void renderCurrentTurn(s_Game* game);
void renderControls(s_Game* game);
void renderEndScreen(s_Game* game, const char won);

void play_render(s_Game* game) {
	renderGrid(game);
	renderCurrentTurn(game);
	renderControls(game);

	if (game->iState == STATE_FINISH_WON) {
		renderEndScreen(game, 1);
	}
	else if (game->iState == STATE_FINISH_LOST) {
		renderEndScreen(game, 0);
	}
}

void renderCurrentTurn(s_Game* game) {
	char score[8];
	int textWidth, textHeight,
		textX, textY,
		widthTextSmall, widthTextLong,
		textMarginRight, textMarginBottom;

	if (IS_GCW) {
		widthTextSmall = 52;
		widthTextLong = 63;
		textMarginRight = 10;
		textMarginBottom = 30;
	}
	else {
		widthTextSmall = 52;
		widthTextLong = 63;
		textMarginRight = 10;
		textMarginBottom = 30;
	}

	snprintf(score, 8, "%d / %d", game->iTurns, MAX_TURNS);

	SDL_Surface* textSurface = TTF_RenderText_Solid(game->scoreFont, score, g_White);
	if (textSurface == NULL) {
		printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
	}
	else {
		SDL_Texture* text = SDL_CreateTextureFromSurface(game->renderer, textSurface);
		textWidth = textSurface->w;
		textHeight = textSurface->h;
		textX = SCREEN_WIDTH - textMarginRight - (game->iTurns < 10 ? widthTextSmall : widthTextLong);
		textY = SCREEN_HEIGHT - textMarginBottom;
		SDL_FreeSurface(textSurface);
		SDL_Rect renderQuad = {textX, textY, textWidth, textHeight};
		SDL_RenderCopy(game->renderer, text, NULL, &renderQuad);
		SDL_DestroyTexture(text);
	}
}

void renderGrid(s_Game* game) {
	int i, j, margin = 1;
	for (j = 0; j < HEIGHT_GRID; ++j){
		for (i = 0; i < WIDTH_GRID; ++i){
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

void renderControls(s_Game* game) {
	int c,
		thicknessSelectedX = (SELECTED_WIDTH_CONTROL_PX - WIDTH_CONTROL_PX) / 2,
		thicknessSelectedY = (SELECTED_HEIGHT_CONTROL_PX - HEIGHT_CONTROL_PX) / 2;
	for (c = 0; c < NB_COLORS; ++c){
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

void renderEndScreen(s_Game* game, const char won) {
	const char *messages[2];
	int textWidth, textHeight, textX, textY, line;

	SDL_Rect bgRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

	SDL_SetRenderDrawBlendMode(game->renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 224);
	SDL_RenderFillRect(game->renderer, &bgRect);

	if (won) {
		messages[0] = "Congratulation!";
	}
	else {
		messages[0] = "You lost.";
	}

	if (IS_GCW) {
		messages[1] = "Click A to restart";
	}
	else {
		messages[1] = "Click SPACE to restart";
	}

	for (line = 0; line < 2; ++line) {
		SDL_Surface* textSurface = TTF_RenderText_Solid(game->endFont, messages[line], g_White);
		if (textSurface == NULL) {
			printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
		}
		else {
			SDL_Texture* text = SDL_CreateTextureFromSurface(game->renderer, textSurface);
			textWidth = textSurface->w;
			textHeight = textSurface->h;
			textX = (SCREEN_WIDTH - textWidth) / 2;
			textY = 50 + line * (textHeight + 5);
			SDL_FreeSurface(textSurface);
			SDL_Rect textRect = {textX, textY, textWidth, textHeight};
			SDL_RenderCopy(game->renderer, text, NULL, &textRect);
			SDL_DestroyTexture(text);
		}
	}
}

void play_handleEvent(s_Game* game, int key) {
	if (
		(IS_GCW && key == SDLK_LCTRL)
		|| (!IS_GCW && key == SDLK_SPACE)
	) {
		play(game);
	}
	// exit if ESCAPE is pressed
	else if (key == SDLK_ESCAPE) {
		game_setFlag(game, FLAG_DONE);
	}
	else if (key == SDLK_UP) {
		game->iSelectedColor = (game->iSelectedColor - 2 + NB_COLORS) % NB_COLORS;
		game_setFlag(game, FLAG_NEEDS_REFRESH);
	}
	else if (key == SDLK_DOWN) {
		game->iSelectedColor = (game->iSelectedColor + 2) % NB_COLORS;
		game_setFlag(game, FLAG_NEEDS_REFRESH);
	}
	else if (key == SDLK_LEFT) {
		game->iSelectedColor = (game->iSelectedColor - 1 + NB_COLORS) % NB_COLORS;
		game_setFlag(game, FLAG_NEEDS_REFRESH);
	}
	else if (key == SDLK_RIGHT) {
		game->iSelectedColor = (game->iSelectedColor + 1) % NB_COLORS;
		game_setFlag(game, FLAG_NEEDS_REFRESH);
	}
}

void play(s_Game* game) {
	if (game->iState != STATE_PLAY) {
		game->iState = STATE_PLAY;
		game_setFlag(game, FLAG_NEEDS_REFRESH);
		game_setFlag(game, FLAG_NEEDS_RESTART);
		return;
	}
	else if (game_selectColor(game)) {
		char finished = game_checkBoard(game);
		if (finished) {
			game->iState = STATE_FINISH_WON;
		}
		else if (game->iTurns == MAX_TURNS) {
			game->iState = STATE_FINISH_LOST;
		}
		else {
			game->iTurns++;
		}

		game_setFlag(game, FLAG_NEEDS_REFRESH);
	}
}
