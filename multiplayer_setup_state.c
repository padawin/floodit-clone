#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "globals.h"
#include "game.h"
#include "multiplayer_setup_state.h"
#include "utils.h"

void multiplayer_setup_state_init(s_Game *game) {

}

void multiplayer_setup_state_clean(s_Game *game) {

}

void multiplayer_setup_render(s_Game* game) {
	printf("Render multi setup\n");
}

void multiplayer_setup_handleEvent(s_Game* game, int key) {
	if (key == SDLK_ESCAPE) {
		multiplayer_setup_state_clean(game);
		game_init(game);
	}
}
