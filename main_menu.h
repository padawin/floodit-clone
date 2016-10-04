#ifndef __MAIN_MENU__
#define __MAIN_MENU__

#include "game.h"
#include "menu.h"

void mainmenu_render(s_Game* game, s_Menu* menu);
void mainmenu_classicMode(s_Game* game);
void mainmenu_timedMode(s_Game* game);
void mainmenu_highScores(s_Game* game);
void mainmenu_quit(s_Game* game);

#endif
