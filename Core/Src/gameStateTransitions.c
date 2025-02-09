#include <stddef.h>
#include "gameStateTransitions.h"

static const StateTransition stateTransitionTable[GAME_STATE_COUNT][EVENT_COUNT] = {
    [GAME_STATE_NONE] = {
        [EVENT_START] = { GAME_STATE_INIT, NULL, 0 },
    },
    [GAME_STATE_INIT] = {
        [EVENT_INITED] = { GAME_STATE_IDLE, NULL, 0 },
    },
    [GAME_STATE_IDLE] = {
        [EVENT_INPUT_RECEIVED] = { GAME_STATE_SHOWING_LEVEL, NULL, 750 },
        [EVENT_STATE_TIMEOUT] = { GAME_STATE_OFF, NULL, 0 },
        [EVENT_INPUT_MENU] = { GAME_STATE_MENU, NULL, 0 },
        [EVENT_BATTERY_LOW] = { GAME_STATE_OFF, NULL, 0 },
    },
    [GAME_STATE_SHOWING_LEVEL] = {
        [EVENT_SEQUENCE_SHOWN] = { GAME_STATE_USER_INPUT, NULL, 0 },
        [EVENT_LEVEL_REPEATED_TOO_MANY_TIMES] = { GAME_STATE_IDLE, NULL, 0 },
        [EVENT_SEQUENCE_CANCELED] = { GAME_STATE_USER_INPUT, NULL, 0 },
        [EVENT_INPUT_MENU] = { GAME_STATE_MENU, NULL, 0 },
        [EVENT_BATTERY_LOW] = { GAME_STATE_OFF, NULL, 0 },
    },
    [GAME_STATE_USER_INPUT] = {
        [EVENT_STATE_TIMEOUT] = { GAME_STATE_SHOWING_LEVEL, NULL, 0 },
        [EVENT_INPUT_CORRECT] = { GAME_STATE_SUCCESS, NULL, 0 },
        [EVENT_INPUT_WRONG] = { GAME_STATE_FAILED, NULL, 0 },
        [EVENT_INPUT_MENU] = { GAME_STATE_MENU, NULL, 0 },
        [EVENT_BATTERY_LOW] = { GAME_STATE_OFF, NULL, 0 },
    },
    [GAME_STATE_SUCCESS] = {
        [EVENT_STATE_TIMEOUT] = { GAME_STATE_SHOWING_LEVEL, NULL, 0 },
    },
    [GAME_STATE_FAILED] = {
        [EVENT_STATE_TIMEOUT] = { GAME_STATE_SHOWING_LEVEL, NULL, 0 },
    },
    [GAME_STATE_OFF] = {
    },
    [GAME_STATE_MENU] = {
        [EVENT_MENU_EXIT] = { GAME_STATE_SHOWING_LEVEL, NULL, 250 },
        [EVENT_STATE_TIMEOUT] = { GAME_STATE_IDLE, NULL, 0 },
        [EVENT_BATTERY_LOW] = { GAME_STATE_OFF, NULL, 0 },
    },
};

const StateTransition * const gameStateGetTransition(GameState state, Event event)
{
    if (state < GAME_STATE_COUNT && event < EVENT_COUNT) {
        return &stateTransitionTable[state][event];
    }
    return NULL;
}
