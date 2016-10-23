#ifndef __MAINMENU_STATE__
#define __MAINMENU_STATE__

#include "game.h"

void mainmenu_state_init(s_Game *game);
void mainmenu_state_handleEvent(s_Game *game, int key);
void mainmenu_state_render(s_Game *game);
void mainmenu_state_clean();

#endif
