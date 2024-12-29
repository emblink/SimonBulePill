#include "stdint.h"
#include "game.h"
#include "notePlayer.h"
#include "effectManager.h"
#include "melodies.h"
#include "keyscan.h"
#include "levels.h"
#include "tim.h"
#include "i2c.h"
#include "oled.h"
#include "gameState.h"

// Time constants
#define SECOND 1000
#define MINUTE (60 * SECOND)

// Timeouts and intervals
#define USER_INPUT_TIMEOUT_MS (10 * SECOND)
#define LEVEL_DISPLAY_RETRIES 3
#define POWER_OFF_TIMEOUT_MS (1 * MINUTE)
#define OLED_REFRESH_INTERVAL_MS 100
#define INIT_STATE_TIMEOUT_MS (1 * SECOND)
#define SUCCESS_STATE_TIMEOUT_MS (500)
#define FAILURE_STATE_TIMEOUT_MS (500)

// Enumerations
typedef enum {
    FontSize12,
    FontSize16,
    FontSize24,
    FontSizeCount,
} FontSize;
typedef union {
    struct {
        uint8_t red : 1;
        uint8_t green : 1;
        uint8_t blue : 1;
        uint8_t yellow : 1;
    };
    uint32_t state;
} Keys;

// TODO: Add game 3 game speeds 1000ms, 500 ms, 250ms
// TODO: Add storage on flash, store current level, speed, game mode, etc
// TODO: Add reset to defaults
// TODO: Add random level mode
// TODO: Add input timeout after second show level run

// State transition table
static uint32_t lastProcessMs = 0;
static uint32_t currentLevel = 1;
static uint32_t levelIdx = 0;
static GameState volatile gameState = GAME_STATE_INIT;
static volatile Keys input = {0};

static const Note keyNoteMap[] = {
    [KEY_RED] = { NOTE_C4, 800 },
    [KEY_GREEN] = { NOTE_E4, 800 },
    [KEY_BLUE] = { NOTE_G4, 800 },
    [KEY_YELLOW] = { NOTE_B4, 800 },
};

static const Led keyLedMap[] = {
    [KEY_RED] = LED_RED,
    [KEY_GREEN] = LED_GREEN,
    [KEY_BLUE] = LED_BLUE,
    [KEY_YELLOW] = LED_YELLOW,
};

static void onPlaybackFinished()
{

}

static void onEffectFinished(Led led)
{

}

static void onKeyPressCallback(Key key, bool isPressed)
{
    switch (key) {
    case KEY_RED: input.red = isPressed; break;
    case KEY_GREEN: input.green = isPressed; break;
    case KEY_BLUE: input.blue = isPressed; break;
    case KEY_YELLOW: input.yellow = isPressed; break;
    default: break;
    }
}

static bool isTimeoutHappened(uint32_t timeoutMs)
{
    uint32_t tick = HAL_GetTick();

    if (tick - lastProcessMs >= timeoutMs) {
        return true;
    }
    return false;
}

static void stateTimeoutProcess()
{
    if (isTimeoutHappened(gameStateGetTimeout())) {
        gameStateProcessEvent(EVENT_STATE_TIMEOUT);
    }
}

static void stateInitEnter()
{
    OLED_Init(&hi2c1);
    OLED_FillScreen(Black);
    OLED_SetCursor(0, 0);
    OLED_SetTextSize(FontSize24);
    OLED_Printf(" SIMON!");
    OLED_UpdateScreen();
    keyscanInit(&onKeyPressCallback);
    notePlayerInit(&onPlaybackFinished);
    effectManagerInit(&onEffectFinished);
    notePlayerPlayMelody(getMelody(MelodyPowerOn), getMelodyLength(MelodyPowerOn));
    effectManagerPlayPowerOn();
}

static void stateInitProcess()
{
    // TODO: show cat wake up animation
    stateTimeoutProcess();
}

static void stateIdleEnter()
{
    effectManagerStopAllEffects();
    effectManagerPlayEffect(EFFECT_BREATHE, LED_GREEN, 0, 2000);
    OLED_FillScreen(Black);
    OLED_SetCursor(0, 0);
    OLED_SetTextSize(FontSize24);
    // TODO: show cat sleep animation
    OLED_Printf("  IDLE");
    input.state = 0; // reset input state
}

static void stateIdleProcess()
{
    stateTimeoutProcess();

    if (input.green) {
        gameStateProcessEvent(EVENT_INPUT_RECEIVED);
    }
}

static void oledShowSequence(const char stateStr[])
{
    OLED_FillScreen(Black);
    OLED_SetCursor(0, 0);
    OLED_SetTextSize(FontSize12);
    OLED_Printf("Lvl:%i%i/%i%i\n", currentLevel / 10, currentLevel % 10,
                LEVELS_COUNT / 10, LEVELS_COUNT % 10);
    OLED_Printf("%s:%i%i/%i%i", stateStr, levelIdx / 10, levelIdx % 10,
                currentLevel / 10, currentLevel % 10);
    OLED_UpdateScreen();
}

static void stateShowLevelEnter()
{
    // Skip level show if some key was pressed
    effectManagerStopAllEffects();
    levelIdx = 0;
    oledShowSequence("Showing");
}

static void stateShowLevelExit()
{
    // Confirm readiness for input with a melody
    effectManagerStopAllEffects();
    notePlayerPlayMelody(getMelody(MelodyConfirm), getMelodyLength(MelodyConfirm));
    levelIdx = 0;
}

static void stateShowLevelProcess()
{
    if (input.state) {
        gameStateProcessEvent(EVENT_SEQUENCE_CANCELED);
    }

    if (effectManagerIsPlaying()) {
        return;
    }

    if (levelIdx >= currentLevel) {
        gameStateProcessEvent(EVENT_SEQUENCE_SHOWN);
    } else {
        // Show the level sequence
        oledShowSequence("Showing");
        Key key = levels[levelIdx++];
        Note n = keyNoteMap[key];
        notePlayerPlayNote(n.note, n.duration);
        effectManagerPlayEffect(EFFECT_FAST_RUMP, keyLedMap[key], n.duration, n.duration);
    }
}

static void stateUserInputEnter()
{
    // Reset input state before the level sequence is shown
    // User have to press a key to cancel the sequence and confirm readiness
    input.state = 0;
    oledShowSequence("Input");
}

static void stateUserInputProcess()
{
    stateTimeoutProcess();

    if (0 == input.state) {
        return;
    }

    lastProcessMs = HAL_GetTick();

    Keys userInput = input; // copy state, should be atomic operation
    input.state &= ~userInput.state; // reset input state bits
    const Key levelKey = levels[levelIdx];
    if (userInput.state & (1 << levelKey)) {
        levelIdx++;
        oledShowSequence("Input");
        Note n = keyNoteMap[levelKey];
        notePlayerPlayNote(n.note, n.duration);
        effectManagerPlayEffect(EFFECT_FAST_RUMP, keyLedMap[levelKey], n.duration, n.duration);
        if (levelIdx >= currentLevel) {
            gameStateProcessEvent(EVENT_INPUT_CORRECT);
        }
    } else {
        // wrong key was pressed
        gameStateProcessEvent(EVENT_INPUT_WRONG);
    }
}

static void stateSuccessEnter()
{
    lastProcessMs = HAL_GetTick();
    effectManagerStopAllEffects();
    effectManagerPlayEffect(EFFECT_BLINK, LED_ALL, 500, 500 / 4);
    notePlayerPlayMelody(getMelody(MelodySuccess), getMelodyLength(MelodySuccess));
    currentLevel++;
    if (currentLevel >= LEVELS_COUNT) {
        // TODO: indicate
        currentLevel = 1;
    }
    levelIdx = 0;
}

static void stateSuccessProcess()
{
    // TODO: maybe process animation
    stateTimeoutProcess();
}

static void stateFailEnter()
{
    effectManagerStopAllEffects();
    effectManagerPlayEffect(EFFECT_BLINK, LED_RED, 500, 500 / 4);
    notePlayerPlayMelody(getMelody(MelodyFail), getMelodyLength(MelodyFail));
    // TODO: show sad cat
    OLED_SetTextSize(FontSize24);
    OLED_Printf("  FAIL :(");
    lastProcessMs = HAL_GetTick();
}

static void stateFailProcess()
{
    // TODO: maybe process animation
    stateTimeoutProcess();
}

static void statePowerOffEnter()
{
    // OLED_FillScreen(Black);
    // OLED_UpdateScreen();
    // OLED_DisplayOff();
    effectManagerPlayPowerOff();
    // TODO: power off animation and sound
    // Shut down MCU
}

void gameInit()
{
    // State definition table
    static const GameStateDef stateDefs[] = {
        [GAME_STATE_NONE]		   = {NULL, NULL, NULL, 0},
        [GAME_STATE_INIT]          = {stateInitEnter, NULL , stateInitProcess, INIT_STATE_TIMEOUT_MS},
        [GAME_STATE_IDLE]          = {stateIdleEnter, NULL, stateIdleProcess, POWER_OFF_TIMEOUT_MS},
        [GAME_STATE_SHOWING_LEVEL] = {stateShowLevelEnter, stateShowLevelExit, stateShowLevelProcess, 0},
        [GAME_STATE_USER_INPUT]    = {stateUserInputEnter, NULL, stateUserInputProcess, USER_INPUT_TIMEOUT_MS},
        [GAME_STATE_SUCCESS]       = {stateSuccessEnter, stateSuccessProcess, NULL, SUCCESS_STATE_TIMEOUT_MS},
        [GAME_STATE_FAILED]        = {stateFailEnter, stateFailProcess, NULL, FAILURE_STATE_TIMEOUT_MS},
        [GAME_STATE_OFF]           = {statePowerOffEnter, NULL, NULL, 0},
    };
    gameStateInit(stateDefs);
    gameStateProcessEvent(EVENT_START);
}

void gameProcess()
{
    gameStateProcess();
    effectManagerProcess();
}
