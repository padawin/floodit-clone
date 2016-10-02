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
	// exit if ESCAPE is pressed
	else if (key == SDLK_ESCAPE) {
		game_setFlag(game, FLAG_DONE);
	}
	else if (key == SDLK_UP) {
		menu->iSelectedItem = (menu->iNbItems + menu->iSelectedItem - 1) % menu->iNbItems;
	}
	else if (key == SDLK_DOWN) {
		menu->iSelectedItem = (menu->iSelectedItem + 1) % menu->iNbItems;
	}
}

void menu_setActionsNumber(s_Menu* menu, int nbActions) {
	menu_free(menu);
	menu->iNbItems = nbActions;
	menu->iNbActionsSet = 0;
	menu->iSelectedItem = 0;
	menu->pActions = (void (**)(s_Game*)) malloc(nbActions * sizeof(void (*)(s_Game*)));
	menu->pItems = (SDL_Texture **) malloc(nbActions * sizeof(SDL_Texture *));
	menu->pSelectedItems = (SDL_Texture **) malloc(nbActions * sizeof(SDL_Texture *));
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
