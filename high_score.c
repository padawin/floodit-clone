#include <libgen.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "high_score.h"

void high_score_save(const int time, const int turns) {
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

	sprintf(filePath, "%s/%s", getenv("HOME"), HIGH_SCORES_LOCATION);
	FILE *f = fopen(filePath, "a");
	if (f == NULL) {
		printf("Error opening file %s\n", filePath);
		return;
	}

	fprintf(f, "%d %d\n", time, turns);
	fclose(f);
}
