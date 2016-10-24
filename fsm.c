#include "fsm.h"

void (*fsm_state_callbacks[][5])() = {
	{
		mainmenu_state_init,
		mainmenu_state_handleEvent,
		0,
		mainmenu_state_render,
		mainmenu_state_clean
	},
	{
		play_state_init,
		play_state_handleEvent,
		0,
		play_state_render,
		play_state_clean
	},
	{
		high_scores_state_init,
		high_scores_state_handleEvent,
		0,
		high_scores_state_render,
		high_scores_state_clean
	},
	{
		multiplayer_setup_state_init,
		multiplayer_setup_state_handleEvent,
		multiplayer_setup_state_update,
		multiplayer_setup_state_render,
		multiplayer_setup_state_clean
	},
	// end state
	{0, 0, 0, 0, 0}
};

s_StateMachine g_stateMachine;

void _executeCallback(s_Game *game, enum fsm_state_action action);

void _executeCallback(s_Game *game, enum fsm_state_action action) {
	if (fsm_state_callbacks[g_stateMachine.currentState][action] != 0) {
		fsm_state_callbacks[g_stateMachine.currentState][action](game);
	}
}

void fsm_init(s_Game *game) {
	g_stateMachine.currentState = FSM_ENTRY_STATE;
	fsm_initState(game);
}

void fsm_setState(s_Game *game, enum fsm_state_code state) {
	fsm_clean(game);
	g_stateMachine.currentState = state;
	fsm_initState(game);
}

void fsm_initState(s_Game *game) {
	_executeCallback(game, fsm_state_init);
}

void fsm_handleEvent(s_Game *game, int key) {
	fsm_state_callbacks[g_stateMachine.currentState][fsm_state_handleEvent](game, key);
}

void fsm_update(s_Game *game) {
	_executeCallback(game, fsm_state_update);
}

void fsm_render(s_Game *game) {
	_executeCallback(game, fsm_state_render);
}

void fsm_clean(s_Game *game) {
	_executeCallback(game, fsm_state_clean);
}

char fsm_isRunning() {
	return g_stateMachine.currentState != FSM_EXIT_STATE;
}
