#ifndef __MULTIPLAYER_SETUP_STATE__
#define __MULTIPLAYER_SETUP_STATE__

void multiplayer_setup_state_init(s_Game *game);
void multiplayer_setup_state_clean(s_Game *game);
void multiplayer_setup_update(s_Game* game);
void multiplayer_setup_render(s_Game* game);
void multiplayer_setup_handleEvent(s_Game* game, int key);

#endif
