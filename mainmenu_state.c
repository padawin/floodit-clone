#include "mainmenu_state.h"
#include "main_menu.h"
#include "utils.h"
#include "menu.h"

SDL_Texture *classicModeTexture, *selectedClassicModeTexture,
	*timedModeTexture, *selectedTimedModeTexture,
	*multiplayerModeTexture, *selectedMultiplayerModeTexture,
	*highScoresTexture, *selectedHighScoresTexture,
	*quitTexture, *selectedQuitTexture;

s_Menu g_mainMenu;

void mainmenu_state_init(s_Game *game) {
	menu_setActionsNumber(&g_mainMenu, 5);
	SDL_Color white = {255, 255, 255};
	utils_createTextTexture(game->renderer, game->menuFont, "Classic Mode", white, &classicModeTexture);
	utils_createTextTexture(game->renderer, game->selectedMenuFont, "Classic Mode", white, &selectedClassicModeTexture);
	utils_createTextTexture(game->renderer, game->menuFont, "Timed Mode", white, &timedModeTexture);
	utils_createTextTexture(game->renderer, game->selectedMenuFont, "Timed Mode", white, &selectedTimedModeTexture);
	utils_createTextTexture(game->renderer, game->menuFont, "Multiplayer Mode", white, &multiplayerModeTexture);
	utils_createTextTexture(game->renderer, game->selectedMenuFont, "Multiplayer Mode", white, &selectedMultiplayerModeTexture);
	utils_createTextTexture(game->renderer, game->menuFont, "High Scores", white, &highScoresTexture);
	utils_createTextTexture(game->renderer, game->selectedMenuFont, "High Scores", white, &selectedHighScoresTexture);
	utils_createTextTexture(game->renderer, game->menuFont, "Quit", white, &quitTexture);
	utils_createTextTexture(game->renderer, game->selectedMenuFont, "Quit", white, &selectedQuitTexture);
	menu_addAction(&g_mainMenu, mainmenu_classicMode, classicModeTexture, selectedClassicModeTexture);
	menu_addAction(&g_mainMenu, mainmenu_timedMode, timedModeTexture, selectedTimedModeTexture);
	menu_addAction(&g_mainMenu, mainmenu_multiplayerMode, multiplayerModeTexture, selectedMultiplayerModeTexture);
	menu_addAction(&g_mainMenu, mainmenu_highScores, highScoresTexture, selectedHighScoresTexture);
	menu_addAction(&g_mainMenu, mainmenu_quit, quitTexture, selectedQuitTexture);
}

void mainmenu_state_handleEvent(s_Game* game, int key) {
	menu_handleEvent(game, &g_mainMenu, key);
}

void mainmenu_state_render(s_Game *game) {
	menu_render(game, &g_mainMenu);
}

void mainmenu_state_clean() {
	menu_free(&g_mainMenu);
}
