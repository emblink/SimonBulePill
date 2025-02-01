#pragma once
#include "gameStateDefines.h"

void gameStateInit(const GameStateDef *stateDefs);
void gameStateProcessEvent(Event event);
void gameStateProcess(void);
GameState gameStateGetCurrentState();
uint32_t gameStateGetNextProcessTime();
