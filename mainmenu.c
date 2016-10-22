#include <stdio.h>
#include "mainmenu.h"
#include "utils.h"
#include "high_score.h"
#include "high_scores_state.h"
#include "play_state.h"
#include "multiplayer_setup_state.h"

void mainmenu_classicMode(s_Game* game) {
	printf("Classic\n");
	play_state_init(game);
	game_start(game, MODE_CLASSIC);
}

void mainmenu_timedMode(s_Game* game) {
	printf("Timed\n");
	play_state_init(game);
	game_start(game, MODE_TIMED);
}

void mainmenu_multiplayerMode(s_Game* game) {
	printf("Multiplayer\n");
	multiplayer_setup_state_init(game);
	game->iState = STATE_MULTIPLAYER_SETUP;
}

void mainmenu_highScores(s_Game* game) {
	high_scores_state_init(game);
	game->iState = STATE_HIGH_SCORES;
}

void mainmenu_quit(s_Game* game) {
	printf("Quit\n");
	game_setFlag(game, FLAG_DONE);
}
