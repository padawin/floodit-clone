#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "menu.h"

void menu_render(s_Game* game, s_Menu* menu) {
	printf("Selected item: %d - %s\n", menu->iSelectedItem, menu->pItems[menu->iSelectedItem]);
}

void menu_handleEvent(s_Game *game, s_Menu* menu, int key) {
	if (
		(IS_GCW && key == SDLK_LCTRL)
		|| (!IS_GCW && key == SDLK_SPACE)
	) {
		menu->pActions[menu->iSelectedItem](game);
		game->cFlags |= FLAG_NEEDS_REFRESH;
	}
	// exit if ESCAPE is pressed
	else if (key == SDLK_ESCAPE) {
		game->cFlags |= FLAG_DONE;
	}
	else if (key == SDLK_UP) {
		menu->iSelectedItem = (menu->iNbItems + menu->iSelectedItem - 1) % menu->iNbItems;
		game->cFlags |= FLAG_NEEDS_REFRESH;
	}
	else if (key == SDLK_DOWN) {
		menu->iSelectedItem = (menu->iSelectedItem + 1) % menu->iNbItems;
		game->cFlags |= FLAG_NEEDS_REFRESH;
	}
}

void menu_setActionsNumber(s_Menu* menu, int nbActions) {
	menu_free(menu);
	menu->iNbItems = nbActions;
	menu->iNbActionsSet = 0;
	menu->iSelectedItem = 0;
	menu->pItems = (const char **) malloc(nbActions * sizeof(const char *));
	menu->pActions = (void (**)(s_Game*)) malloc(nbActions * sizeof(void (*)(s_Game*)));
}

void menu_addAction(s_Menu* menu, const char *actionLabel, void (*pAction)(s_Game*)) {
	menu->pItems[menu->iNbActionsSet] = actionLabel;
	menu->pActions[menu->iNbActionsSet] = pAction;
	menu->iNbActionsSet++;
}

void menu_free(s_Menu* menu) {
	free(menu->pItems);
	free(menu->pActions);
}
