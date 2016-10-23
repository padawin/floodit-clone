#ifndef __FSM__
#define __FSM__

#include "mainmenu_state.h"
#include "play_state.h"
#include "high_scores_state.h"
#include "multiplayer_setup_state.h"

enum fsm_state_action {fsm_state_init, fsm_state_handleEvent, fsm_state_update, fsm_state_render, fsm_state_clean};
enum fsm_state_code {mainmenu, play, high_scores, multiplayer_setup, end};
enum fsm_return_code {ok, back, quit, repeat, menu};

#define FSM_NB_STATES_TRANSITIONS 9

#define FSM_EXIT_STATE end
#define FSM_ENTRY_STATE mainmenu

typedef struct {
	enum fsm_state_code currentState;
} s_StateMachine;

void fsm_init(s_Game *game);
void fsm_setState(s_Game *game, enum fsm_state_code state);
void fsm_initState(s_Game *game);
void fsm_handleEvent(s_Game *game, int key);
void fsm_update(s_Game *game);
void fsm_render(s_Game *game);
void fsm_clean(s_Game *game);
char fsm_isRunning();

#endif
