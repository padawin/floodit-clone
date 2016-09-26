#ifndef __MENU__
#define __MENU__

#include "game.h"

typedef struct {
	const char** pItems;
	void (**pActions)(s_Game*);
	int iNbItems;
	int iNbActionsSet;
	int iSelectedItem;
} s_Menu;

void menu_handleEvent(s_Game* game, s_Menu* menu, int key);
void menu_setActionsNumber(s_Menu* menu, int nbActions);
void menu_addAction(s_Menu* menu, const char *actionLabel, void (*pAction)(s_Game*));
void menu_free();

#endif
