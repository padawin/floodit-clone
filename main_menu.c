#include <stdio.h>
#include "main_menu.h"

void mainmenu_render(s_Game* game, s_Menu* menu) {
	printf("Selected item: %d - %s\n", menu->iSelectedItem, menu->pItems[menu->iSelectedItem]);
}

void mainmenu_normalMode(s_Game* game) {
	printf("Normal\n");
	game->iState = STATE_PLAY;
}

void mainmenu_quit(s_Game* game) {
	printf("Quit\n");
	game_setFlag(game, FLAG_DONE);
}
