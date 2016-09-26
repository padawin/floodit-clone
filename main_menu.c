#include <stdio.h>
#include "main_menu.h"

void mainmenu_normalMode(s_Game* game) {
	printf("Normal\n");
	game->iState = STATE_PLAY;
}

void mainmenu_quit(s_Game* game) {
	printf("Quit\n");
}
