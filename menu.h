#ifndef __MENU__
#define __MENU__

#include <SDL2/SDL.h>
#include "game.h"

typedef struct {
	void (**pActions)(s_Game*);
	SDL_Texture **pItems;
	SDL_Texture **pSelectedItems;
	int iNbItems;
	int iNbActionsSet;
	int iSelectedItem;
} s_Menu;

void menu_handleEvent(s_Game* game, s_Menu* menu, int key);
void menu_render(s_Game* game, s_Menu* menu);
void menu_setActionsNumber(s_Menu* menu, int nbActions);
void menu_addAction(
	s_Menu* menu,
	void (*pAction)(s_Game*),
	SDL_Texture *itemTexture,
	SDL_Texture *selectedItemTexture
);
void menu_free();

#endif
