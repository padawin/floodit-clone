#include <SDL2/SDL.h>
#include "globals.h"
#include "game.h"
#include "high_scores_state.h"
#include "high_score.h"

void high_scores_render(s_Game* game) {
	int times[MAX_HIGH_SCORES_NUMBER];
	int turns[MAX_HIGH_SCORES_NUMBER];
	int nbRows;
	high_score_list(times, turns, &nbRows);
	int i;
	printf("High Scores:\n");
	for (i = 0; i < MAX_HIGH_SCORES_NUMBER; ++i) {
		if (i < nbRows) {
			printf("Time: %d, Turns: %d\n", times[i], turns[i]);
		}
		else {
			printf("Time: -, Turns: -\n");
		}
	}
}

void high_scores_handleEvent(s_Game* game, int key) {
	// exit if ESCAPE is pressed
	if (key == SDLK_ESCAPE) {
		game_init(game);
	}
}
