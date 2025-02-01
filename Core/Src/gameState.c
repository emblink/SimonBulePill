#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "gameState.h"
#include "gameStateTransitions.h"
#include "stm32f1xx_hal.h"

static const GameStateDef *stateDefs = NULL;
static GameState currentState = GAME_STATE_NONE;
static GameState nextState = GAME_STATE_NONE;
static uint32_t transitionStartMs = 0;
static uint32_t transitionTimeoutMs = 0;
static uint32_t stateEnterMs = 0;

static void changeState(const StateTransition *transition)
{
    if (stateDefs[currentState].onExit) {
        stateDefs[currentState].onExit();
    }

    if (transition->delayMs) {
        nextState = transition->nextState;
        currentState = GAME_STATE_TRANSITION;
        transitionStartMs = HAL_GetTick();
        transitionTimeoutMs = transition->delayMs;
    } else {
        currentState = transition->nextState;
        if (stateDefs[currentState].onEnter) {
            stateEnterMs = HAL_GetTick();
            stateDefs[currentState].onEnter();
        }
    }
}

static uint32_t gameStateGetTimeout(void)
{
    return stateDefs[currentState].timeoutMs;
}

static inline bool isTimeoutHappened(uint32_t lastProcessMs, uint32_t timeoutMs)
{
    return HAL_GetTick() - lastProcessMs >= timeoutMs;
}

static void gameStateProcessTimeout()
{
    uint32_t stateTimeoutMs = gameStateGetTimeout();
    if (stateTimeoutMs && isTimeoutHappened(stateEnterMs, stateTimeoutMs)) {
        gameStateProcessEvent(EVENT_STATE_TIMEOUT);
    }
}

void gameStateInit(const GameStateDef *states)
{
    stateDefs = states;
}

void gameStateProcessEvent(Event event)
{
    // Process current state event
    const StateTransition *transition = gameStateGetTransition(currentState, event);
    if (transition) {
        changeState(transition);
    }
}

void gameStateProcess(void)
{
    if (GAME_STATE_TRANSITION == currentState) {
        if (HAL_GetTick() - transitionStartMs >= transitionTimeoutMs) {
            currentState = nextState;
            if (stateDefs[currentState].onEnter) {
            	stateEnterMs = HAL_GetTick();
                stateDefs[currentState].onEnter();
            }
        }
    }

    if (stateDefs[currentState].onProcess) {
        stateDefs[currentState].onProcess();
    }
    gameStateProcessTimeout();
}

GameState gameStateGetCurrentState()
{
    return currentState;
}

uint32_t gameStateGetNextProcessTime()
{
    uint32_t currentTick = HAL_GetTick();
    if (GAME_STATE_TRANSITION == currentState) {
        return transitionStartMs + transitionTimeoutMs;
    }

    uint32_t stateTimeout = gameStateGetTimeout();
    if (stateTimeout) {
        return stateEnterMs + stateTimeout;
    }

    return 0;
}
