#include <SDL2/SDL.h>
#include "globals.h"
#include "game.h"
#include "high_scores_state.h"
#include "high_score.h"
#include "utils.h"

SDL_Texture *highScoresTitle;
SDL_Texture *highScoreTextures[MAX_HIGH_SCORES_NUMBER][2];
void high_scores_state_clean(s_Game *game);

void high_scores_state_init(s_Game *game) {
	int times[MAX_HIGH_SCORES_NUMBER];
	int turns[MAX_HIGH_SCORES_NUMBER];
	int nbRows, i;
	SDL_Color white = {255, 255, 255};
	utils_createTextTexture(
		game->renderer,
		game->highScoreTitleFont,
		"High Scores",
		white,
		&highScoresTitle
	);

	high_score_list(times, turns, &nbRows);
	for (i = 0; i < MAX_HIGH_SCORES_NUMBER; ++i) {
		char timeLabel[12], turnsLabel[10];
		if (i < nbRows) {
			snprintf(timeLabel, 12, "Time: %d", times[i]);
			snprintf(turnsLabel, 10, "Turns: %d", turns[i]);
		}
		else {
			snprintf(timeLabel, 8, "Time: -");
			snprintf(turnsLabel, 9, "Turns: -");
		}

		utils_createTextTexture(
			game->renderer,
			game->highScoreFont,
			timeLabel,
			white,
			&highScoreTextures[i][0]
		);
		utils_createTextTexture(
			game->renderer,
			game->highScoreFont,
			turnsLabel,
			white,
			&highScoreTextures[i][1]
		);
	}
}

void high_scores_state_clean(s_Game *game) {
	int i;

	SDL_DestroyTexture(highScoresTitle);
	for (i = 0; i < MAX_HIGH_SCORES_NUMBER; ++i) {
		SDL_DestroyTexture(highScoreTextures[i][0]);
		SDL_DestroyTexture(highScoreTextures[i][1]);
	}
}

void high_scores_render(s_Game* game) {
	int i, textWidth, textHeight,
		marginTitleX, marginTitleY,
		marginTimeX, marginTurnsX, marginY;

	if (IS_GCW) {
		marginTitleX = 20;
		marginTitleY = 10;
		marginTimeX = 20;
		marginTurnsX = 160;
		marginY = 50;
	}
	else {
		marginTitleX = 50;
		marginTitleY = 50;
		marginTimeX = 50;
		marginTurnsX = 200;
		marginY = 100;
	}

	SDL_QueryTexture(highScoresTitle, NULL, NULL, &textWidth, &textHeight);
	SDL_Rect renderQuad = {marginTitleX, marginTitleY, textWidth, textHeight};
	SDL_RenderCopy(game->renderer, highScoresTitle, NULL, &renderQuad);

	for (i = 0; i < MAX_HIGH_SCORES_NUMBER; ++i) {
		SDL_QueryTexture(highScoreTextures[i][0], NULL, NULL, &textWidth, &textHeight);
		SDL_Rect renderQuadTime = {marginTimeX, marginY + 33 * i, textWidth, textHeight};
		SDL_RenderCopy(game->renderer, highScoreTextures[i][0], NULL, &renderQuadTime);
		SDL_QueryTexture(highScoreTextures[i][1], NULL, NULL, &textWidth, &textHeight);
		SDL_Rect renderQuadTurns = {marginTurnsX, marginY + 33 * i, textWidth, textHeight};
		SDL_RenderCopy(game->renderer, highScoreTextures[i][1], NULL, &renderQuadTurns);
	}
}

void high_scores_handleEvent(s_Game* game, int key) {
	// exit if ESCAPE is pressed
	if (key == SDLK_ESCAPE) {
		high_scores_state_clean(game);
		game_init(game);
	}
}
