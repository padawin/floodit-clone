#ifndef __MULTIPLAYER_SETUP_STATE__
#define __MULTIPLAYER_SETUP_STATE__

void multiplayer_setup_state_init(s_Game *game);
void multiplayer_setup_state_update(s_Game* game);
void multiplayer_setup_state_render(s_Game* game);
void multiplayer_setup_state_handleEvent(s_Game* game, int key);
void multiplayer_setup_state_clean();

#endif
