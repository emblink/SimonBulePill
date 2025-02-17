#pragma once
#include "gameStateDefines.h"

void gameStateInit(const GameStateDef *stateDefs);
void gameStateProcessEvent(Event event);
void gameStateProcessEventWithDelay(Event event, uint32_t delayMs);
void gameStateProcess(void);
GameState gameStateGetCurrentState();
uint32_t gameStateGetNextProcessInterval();
void gameStateResetTimeout();
