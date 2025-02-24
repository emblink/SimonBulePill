#include <stddef.h>
#include "sceneSimonStates.h"
#include "sceneSimonTransitions.h"

static const StateTransition sceneSimonTransitionTable[GAME_STATE_COUNT][EVENT_COUNT] = {
    [GAME_STATE_INIT] = {
        [EVENT_STATE_TIMEOUT] = { GAME_STATE_SHOWING_LEVEL, NULL, 0 },
    },
    [GAME_STATE_SHOWING_LEVEL] = {
        [EVENT_SEQUENCE_SHOWN] = { GAME_STATE_USER_INPUT, NULL, 0 },
        [EVENT_LEVEL_REPEATED_TOO_MANY_TIMES] = { GAME_STATE_IDLE, NULL, 0 },
        [EVENT_SEQUENCE_CANCELED] = { GAME_STATE_USER_INPUT, NULL, 0 },
    },
    [GAME_STATE_USER_INPUT] = {
        [EVENT_STATE_TIMEOUT] = { GAME_STATE_SHOWING_LEVEL, NULL, 0 },
        [EVENT_INPUT_CORRECT] = { GAME_STATE_SUCCESS, NULL, 0 },
        [EVENT_INPUT_WRONG] = { GAME_STATE_FAILED, NULL, 0 },
    },
    [GAME_STATE_SUCCESS] = {
        [EVENT_STATE_TIMEOUT] = { GAME_STATE_SHOWING_LEVEL, NULL, 0 },
    },
    [GAME_STATE_FAILED] = {
        [EVENT_STATE_TIMEOUT] = { GAME_STATE_SHOWING_LEVEL, NULL, 0 },
    },
    [GAME_STATE_IDLE] = {

    },
};

const StateTransition * const gameStateGetTransition(uint32_t state, uint32_t event)
{
    if (state < GAME_STATE_COUNT && event < EVENT_COUNT) {
        return &sceneSimonTransitionTable[state][event];
    }
    return NULL;
}
