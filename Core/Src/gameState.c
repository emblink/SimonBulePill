#include "stddef.h"
#include "gameStateTransitions.h"
#include "stm32f1xx_hal.h"

static const GameStateDef *stateDefs = NULL;
static GameState currentState = GAME_STATE_NONE;
static GameState nextState = GAME_STATE_NONE;
static uint32_t transitionStartMs = 0;
static uint32_t transitionTimeoutMs = 0;

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
            stateDefs[currentState].onEnter();
        }
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
                stateDefs[currentState].onEnter();
            }
        }
    }

    if (stateDefs[currentState].onProcess) {
        stateDefs[currentState].onProcess();
    }
}

uint32_t gameStateGetTimeout(void)
{
    return stateDefs[currentState].timeoutMs;
}

GameState gameStateGet()
{
    return currentState;
}
