
#include "gameStateDefines.h"

void gameStateInit(const GameStateDef *stateDefs);
void gameStateProcessEvent(Event event);
void gameStateProcess(void);
uint32_t gameStateGetTimeout(void);
GameState gameStateGet();
