#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "menu.h"

void menu_handleEvent(s_Game *game, s_Menu* menu, int key) {
	if (
		(IS_GCW && key == SDLK_LCTRL)
		|| (!IS_GCW && key == SDLK_SPACE)
	) {
		menu->pActions[menu->iSelectedItem](game);
	}
	else if (key == SDLK_UP) {
		menu->iSelectedItem = (menu->iNbItems + menu->iSelectedItem - 1) % menu->iNbItems;
	}
	else if (key == SDLK_DOWN) {
		menu->iSelectedItem = (menu->iSelectedItem + 1) % menu->iNbItems;
	}
}

void menu_render(s_Game* game, s_Menu* menu) {
	int i;
	for (i = 0; i < menu->iNbItems; ++i) {
		SDL_Texture *item;
		if (i == menu->iSelectedItem) {
			item = menu->pSelectedItems[i];
		}
		else {
			item = menu->pItems[i];
		}

		int textWidth, textHeight;
		SDL_QueryTexture(item, NULL, NULL, &textWidth, &textHeight);
		SDL_Rect renderQuad = {50, 30 + i * 35, textWidth, textHeight};
		SDL_RenderCopy(game->renderer, item, NULL, &renderQuad);
	}
}

void menu_setActionsNumber(s_Menu* menu, int nbActions) {
	menu->iNbItems = nbActions;
	menu->iNbActionsSet = 0;
	menu->iSelectedItem = 0;
	unsigned long nmemb = (unsigned long) nbActions;
	menu->pActions = (void (**)(s_Game*)) malloc(nmemb * sizeof(void (*)(s_Game*)));
	menu->pItems = (SDL_Texture **) malloc(nmemb * sizeof(SDL_Texture *));
	menu->pSelectedItems = (SDL_Texture **) malloc(nmemb * sizeof(SDL_Texture *));
}

void menu_addAction(
	s_Menu* menu,
	void (*pAction)(s_Game*),
	SDL_Texture *itemTexture,
	SDL_Texture *selectedItemTexture
) {
	menu->pActions[menu->iNbActionsSet] = pAction;
	menu->pItems[menu->iNbActionsSet] = itemTexture;
	menu->pSelectedItems[menu->iNbActionsSet] = selectedItemTexture;
	menu->iNbActionsSet++;
}

void menu_free(s_Menu* menu) {
	int t;
	for (t = 0; t < menu->iNbItems; ++t) {
		SDL_DestroyTexture(menu->pItems[t]);
		SDL_DestroyTexture(menu->pSelectedItems[t]);
	}

	free(menu->pItems);
	free(menu->pSelectedItems);
	free(menu->pActions);
}
