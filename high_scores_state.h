#ifndef __HIGH_SCORE_STATE__
#define __HIGH_SCORE_STATE__

void high_scores_state_init(s_Game *game);
void high_scores_render(s_Game* game);
void high_scores_handleEvent(s_Game* game, int key);
void high_scores_state_clean();

#endif
