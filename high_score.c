#include <libgen.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "high_score.h"
#include <errno.h>
#include <string.h>

void _prepare_folder();
FILE* _open_file(const char *mode);

void high_score_save(const int time, const int turns) {
	FILE *f;
	int highScoreTimes[MAX_HIGH_SCORES_NUMBER];
	int highScoreTurns[MAX_HIGH_SCORES_NUMBER];
	int nbRows;

	_prepare_folder();

	high_score_list(highScoreTimes, highScoreTurns, &nbRows);
	f = _open_file("w");
	if (f == NULL) {
		return;
	}

	int existingScoresCursor = 0,
		newScoresCursor = 0;
	char newScoreInserted = 0;
	while (newScoresCursor < MAX_HIGH_SCORES_NUMBER && newScoresCursor < nbRows + 1) {
		if (!newScoreInserted && (newScoresCursor == nbRows || time < highScoreTimes[existingScoresCursor])) {
			fprintf(f, "%d %d\n", time, turns);
			newScoreInserted = 1;
		}
		else {
			fprintf(
				f, "%d %d\n",
				highScoreTimes[existingScoresCursor],
				highScoreTurns[existingScoresCursor]
			);
			++existingScoresCursor;
		}
		++newScoresCursor;
	}
	fclose(f);
}

void high_score_list(int *times, int *turns, int *nbRows) {
	FILE *f;
	char line[32], *read;

	*nbRows = 0;
	f = _open_file("r");
	if (f == NULL) {
		return;
	}

	while ((read = fgets(line, 32, f)) != NULL) {
		int readTime, readTurns;
		if (-1 != sscanf(line, "%d %d\n", &readTime, &readTurns)) {
			times[*nbRows] = readTime;
			turns[*nbRows] = readTurns;
			*nbRows += 1;
		}
	}

	fclose(f);
}

void _prepare_folder() {
	char filePath[255];

	sprintf(
		filePath,
		"%s/%s",
		getenv("HOME"),
		// dirname needs a copy of the string
		dirname(strdup(HIGH_SCORES_LOCATION))
	);

	struct stat st = {0};
	if (stat(filePath, &st) == -1) {
		printf("Create folder %s\n", filePath);
		if (mkdir(filePath, S_IRWXU | S_IRWXG | S_IRWXO)) {
			printf("Error while creating folder %s\n", filePath);
		}
	}
}

FILE* _open_file(const char *mode) {
	FILE *f;
	char filePath[255];
	sprintf(filePath, "%s/%s", getenv("HOME"), HIGH_SCORES_LOCATION);
	f = fopen(filePath, mode);
	if (f == NULL) {
		printf("Error while reading high scores file %s\n", filePath);
		printf("Reason: %s\n", strerror(errno));
	}

	return f;
}
