#pragma once

typedef enum {
    GAME_STATE_TRANSITION,    // Special state for transition with delay between states
    GAME_STATE_INIT,          // Startup animation and sound
    GAME_STATE_SHOWING_LEVEL, // Display current level (lights/sounds)
    GAME_STATE_USER_INPUT,    // User input sequence verification
    GAME_STATE_SUCCESS,       // Success feedback (animation, sound)
    GAME_STATE_FAILED,        // Failure feedback (animation, sound)
    GAME_STATE_IDLE,          // Exit scene and switch to IDLE scene
    GAME_STATE_COUNT
} SceneState;

typedef enum {
    EVENT_STATE_TIMEOUT,
    EVENT_LEVEL_REPEATED_TOO_MANY_TIMES,
    EVENT_SEQUENCE_SHOWN,
    EVENT_SEQUENCE_CANCELED,
    EVENT_INPUT_RECEIVED,
    EVENT_INPUT_CORRECT,
    EVENT_INPUT_WRONG,
    EVENT_COUNT
} SceneEvent;
