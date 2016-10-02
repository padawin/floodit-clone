#include "utils.h"

int utils_popArray(int* array, int* arrayLength) {
	// swap the first element with the last, reduce the length, return the old
	// first element
	if (arrayLength == 0) {
		return -1;
	}

	int elem = array[0];
	int tmp = array[(*arrayLength) - 1];
	array[(*arrayLength) - 1] = elem;
	array[0] = tmp;
	(*arrayLength) -= 1;
	return elem;
}

void utils_createTextTexture(
	SDL_Renderer *renderer,
	TTF_Font *font,
	const char *text,
	SDL_Color color,
	SDL_Texture **texture
) {
	SDL_Surface* textSurface;
	textSurface = TTF_RenderText_Solid(font, text, color);

	if (textSurface == NULL) {
		printf(
			"Unable to render text surface! SDL_ttf Error: %s\n",
			TTF_GetError()
		);
	}
	else {
		*texture = SDL_CreateTextureFromSurface(renderer, textSurface);
		SDL_FreeSurface(textSurface);
	}
}

void utils_renderText(
	s_Game *game,
	TTF_Font *font,
	const char *text,
	SDL_Color color,
	const int x, const int y
) {
	int textWidth, textHeight;
	SDL_Texture* textTexture = 0;

	utils_createTextTexture(game->renderer, font, text, color, &textTexture);
	if (textTexture != 0) {
		SDL_QueryTexture(textTexture, NULL, NULL, &textWidth, &textHeight);
		SDL_Rect renderQuad = {x, y, textWidth, textHeight};
		SDL_RenderCopy(game->renderer, textTexture, NULL, &renderQuad);
		SDL_DestroyTexture(textTexture);
	}
}
