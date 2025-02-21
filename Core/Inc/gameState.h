#pragma once

// State definitions
typedef struct {
    void (*onEnter)(void);
    void (*onExit)(void);
    void (*onProcess)(void);
    uint32_t timeoutMs;
} GameStateDef;

void gameStateInit(const GameStateDef *stateDefs);
void gameStateProcessEvent(uint32_t event);
void gameStateProcessEventWithDelay(uint32_t event, uint32_t delayMs);
void gameStateProcess(void);
uint32_t gameStateGetCurrentState();
uint32_t gameStateGetNextProcessInterval();
void gameStateResetTimeout();
