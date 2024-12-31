#pragma once
#include <stdint.h>

typedef enum {
    GAME_STATE_NONE,
    GAME_STATE_TRANSITION,
    GAME_STATE_INIT,          // Startup animation and sound
    GAME_STATE_IDLE,          // Idle animation
    GAME_STATE_SHOWING_LEVEL, // Display current level (lights/sounds)
    GAME_STATE_USER_INPUT,    // User input sequence verification
    GAME_STATE_SUCCESS,       // Success feedback (animation, sound)
    GAME_STATE_FAILED,        // Failure feedback (animation, sound)
    GAME_STATE_OFF,
    GAME_STATE_MENU,
    GAME_STATE_COUNT
} GameState;

typedef enum {
    EVENT_START,
    EVENT_INITED,
    EVENT_STATE_TIMEOUT,
    EVENT_SEQUENCE_SHOWN,
    EVENT_SEQUENCE_CANCELED,
    EVENT_INPUT_RECEIVED,
    EVENT_INPUT_TIMEOUT,
    EVENT_INPUT_CORRECT,
    EVENT_INPUT_WRONG,
    EVENT_INPUT_MENU,
    EVENT_MENU_EXIT,
    EVENT_COUNT
} Event;

// State definitions
typedef struct {
    void (*onEnter)(void);
    void (*onExit)(void);
    void (*onProcess)(void);
    uint32_t timeoutMs;
} GameStateDef;
