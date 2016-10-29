#include "mainmenu.h"
#include "fsm.h"

void mainmenu_classicMode(s_Game* game) {
	game_setMode(game, MODE_CLASSIC);
	fsm_setState(game, play);
}

void mainmenu_timedMode(s_Game* game) {
	game_setMode(game, MODE_TIMED);
	fsm_setState(game, play);
}

void mainmenu_multiplayerMode(s_Game* game) {
	fsm_setState(game, multiplayer_setup);
}

void mainmenu_highScores(s_Game* game) {
	fsm_setState(game, high_scores);
}

void mainmenu_quit(s_Game* game) {
	fsm_setState(game, end);
}
