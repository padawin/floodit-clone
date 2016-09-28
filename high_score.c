#include <libgen.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "high_score.h"

FILE* _open_file(const char *mode);

void high_score_save(const int time, const int turns) {
	FILE *f;
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
		mkdir(filePath, 0700);
	}

	f = _open_file("a");
	if (f == NULL) {
		return;
	}

	fprintf(f, "%d %d\n", time, turns);
	fclose(f);
}

FILE* _open_file(const char *mode) {
	FILE *f;
	char filePath[255];
	sprintf(filePath, "%s/%s", getenv("HOME"), HIGH_SCORES_LOCATION);
	f = fopen(filePath, mode);
	if (f == NULL) {
		printf("Error while reading high scores file %s\n", filePath);
	}

	return f;
}
