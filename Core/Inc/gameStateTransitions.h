#pragma once
#include "gameStateDefines.h"

typedef struct {
    GameState nextState;
    void (*action)(void);
    uint32_t delayMs;
} StateTransition;

const StateTransition * const gameStateGetTransition(GameState state, Event event);