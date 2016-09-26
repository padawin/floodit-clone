#include <stdio.h>
#include "main_menu.h"

void mainmenu_render(s_Game* game, s_Menu* menu) {
	SDL_Color white = {255, 255, 255};
	int i, textX, textY, textWidth, textHeight;
	SDL_Surface* textSurface;
	for (i = 0; i < menu->iNbItems; ++i) {
		TTF_Font *font;
		if (i == menu->iSelectedItem) {
			font = game->selectedMenuFont;
		}
		else {
			font = game->menuFont;
		}

		textSurface = TTF_RenderText_Solid(font, menu->pItems[i], white);

		if (textSurface == NULL) {
			printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
		}
		else {
			SDL_Texture* text = SDL_CreateTextureFromSurface(game->renderer, textSurface);
			textWidth = textSurface->w;
			textHeight = textSurface->h;
			textX = 50;
			textY = 50 + i * 30;
			SDL_FreeSurface(textSurface);
			SDL_Rect renderQuad = {textX, textY, textWidth, textHeight};
			SDL_RenderCopy(game->renderer, text, NULL, &renderQuad);
			SDL_DestroyTexture(text);
		}
	}
}

void mainmenu_normalMode(s_Game* game) {
	printf("Normal\n");
	game->iState = STATE_PLAY;
}

void mainmenu_timedMode(s_Game* game) {
	printf("Timed\n");
}

void mainmenu_quit(s_Game* game) {
	printf("Quit\n");
	game_setFlag(game, FLAG_DONE);
}
