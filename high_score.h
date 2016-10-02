#ifndef __HIGH_SCORE__
#define __HIGH_SCORE__

#define HIGH_SCORES_LOCATION ".floodit/highscores"
#define MAX_HIGH_SCORES_NUMBER 5

void high_score_save(const int time, const int turns);
void high_score_list(int *times, int *turns, int *nbRows);

#endif
