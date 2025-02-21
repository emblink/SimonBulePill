#pragma once
#include <stdint.h>

typedef struct {
    uint32_t nextState;
    void (*action)(void);
    uint32_t delayMs;
} StateTransition;

const StateTransition * const gameStateGetTransition(uint32_t state, uint32_t event);