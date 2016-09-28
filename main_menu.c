#include <stdio.h>
#include "main_menu.h"
#include "utils.h"
#include "high_score.h"

void mainmenu_render(s_Game* game, s_Menu* menu) {
	int i;
	for (i = 0; i < menu->iNbItems; ++i) {
		SDL_Texture *item;
		if (i == menu->iSelectedItem) {
			item = menu->pSelectedItems[i];
		}
		else {
			item = menu->pItems[i];
		}

		int textWidth, textHeight;
		SDL_QueryTexture(item, NULL, NULL, &textWidth, &textHeight);
		SDL_Rect renderQuad = {50, 50 + i * 35, textWidth, textHeight};
		SDL_RenderCopy(game->renderer, item, NULL, &renderQuad);
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

void mainmenu_highScores(s_Game* game) {
	int times[MAX_HIGH_SCORES_NUMBER];
	int turns[MAX_HIGH_SCORES_NUMBER];
	int nbRows;
	high_score_list(times, turns, &nbRows);
	int i;
	for (i = 0; i < nbRows; ++i) {
		printf("Time: %d, Turns: %d\n", times[i], turns[i]);
	}
}

void mainmenu_quit(s_Game* game) {
	printf("Quit\n");
	game_setFlag(game, FLAG_DONE);
}
