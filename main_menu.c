#include <stdio.h>
#include "main_menu.h"
#include "utils.h"

void mainmenu_render(s_Game* game, s_Menu* menu) {
	SDL_Color white = {255, 255, 255};
	int i;
	for (i = 0; i < menu->iNbItems; ++i) {
		TTF_Font *font;
		if (i == menu->iSelectedItem) {
			font = game->selectedMenuFont;
		}
		else {
			font = game->menuFont;
		}

		utils_renderText(game, font, menu->pItems[i], white, 50, 50 + i * 30);
	}
}

void mainmenu_normalMode(s_Game* game) {
	printf("Normal\n");
	game_start(game, MODE_NORMAL);
}

void mainmenu_timedMode(s_Game* game) {
	printf("Timed\n");
	game_start(game, MODE_TIMED);
}

void mainmenu_quit(s_Game* game) {
	printf("Quit\n");
	game_setFlag(game, FLAG_DONE);
}
