#ifndef __PLAY_STATE__
#define __PLAY_STATE__

void play_state_init(s_Game *game);
void play_state_handleEvent(s_Game* game, int key);
void play_state_update(s_Game *game);
void play_state_render(s_Game* game);
void play_state_clean();

#endif
