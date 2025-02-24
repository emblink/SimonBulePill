#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "gameState.h"
#include "sceneSimonStates.h"
#include "sceneSimonTransitions.h"
#include "stm32f1xx_hal.h"
#include "generic.h"

static const GameStateDef *stateDefs = NULL;
static uint32_t currentState = GAME_STATE_INIT;
static uint32_t nextState = GAME_STATE_INIT;
static uint32_t transitionStartMs = 0;
static uint32_t transitionTimeoutMs = 0;
static uint32_t stateEnterMs = 0;

static void changeState(const StateTransition *transition, uint32_t delayMs)
{
    if (stateDefs[currentState].onExit) {
        stateDefs[currentState].onExit();
    }

    if (delayMs) {
        nextState = transition->nextState;
        currentState = GAME_STATE_TRANSITION;
        transitionStartMs = HAL_GetTick();
        transitionTimeoutMs = delayMs;
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
    currentState = GAME_STATE_INIT;
    nextState = GAME_STATE_INIT;
    transitionStartMs = 0;
    transitionTimeoutMs = 0;
    stateEnterMs = 0;
    if (stateDefs[currentState].onEnter) {
        stateEnterMs = HAL_GetTick();
        stateDefs[currentState].onEnter();
    }
}

void gameStateProcessEvent(uint32_t event)
{
    // Process current state event
    const StateTransition *transition = gameStateGetTransition(currentState, event);
    if (transition) {
        changeState(transition, transition->delayMs);
    }
}

void gameStateProcessEventWithDelay(uint32_t event, uint32_t delayMs)
{
    // Process current state event
    const StateTransition *transition = gameStateGetTransition(currentState, event);
    if (transition) {
        changeState(transition, delayMs);
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

uint32_t gameStateGetCurrentState()
{
    return currentState;
}

uint32_t gameStateGetNextProcessInterval()
{
    uint32_t currentTick = HAL_GetTick();
    uint32_t nextProcessTick = 0;
    if (GAME_STATE_TRANSITION == currentState) {
        nextProcessTick = transitionStartMs + transitionTimeoutMs;
    }

    uint32_t stateTimeout = gameStateGetTimeout();
    if (stateTimeout) {
        nextProcessTick = stateEnterMs + stateTimeout;
    }

    if (0 == nextProcessTick) {
        return 0;
    }

    return nextProcessTick - currentTick;
}

void gameStateResetTimeout()
{
    stateEnterMs = HAL_GetTick();
}