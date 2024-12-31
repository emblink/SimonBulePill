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
#include "gameSettings.h"
#include "fontSize.h"
#include "gameMenu.h"

// Time constants
#define SECOND 1000
#define MINUTE (60 * SECOND)

// Timeouts and intervals
#define USER_INPUT_TIMEOUT_MS (10 * SECOND)
#define LEVEL_SHOW_RETRIES 3
#define POWER_OFF_TIMEOUT_MS (1 * MINUTE)
#define OLED_REFRESH_INTERVAL_MS 100
#define INIT_STATE_TIMEOUT_MS (1 * SECOND)
#define SUCCESS_STATE_TIMEOUT_MS (750)
#define FAILURE_STATE_TIMEOUT_MS (750)
#define MENU_TIMEOUT_MS (1 * MINUTE)

typedef union {
    struct {
        uint8_t red : 1;
        uint8_t green : 1;
        uint8_t blue : 1;
        uint8_t yellow : 1;
        uint8_t menu : 1;
    };
    uint32_t state;
} Keys;

// TODO: Add a menu button handling
// TODO: Add a in game menu state with settings
// TODO: Add 1 min timeut for the menu state with idle transition
// TODO: Add level, speed, mode, sequence, reset to defaults menu list
// TODO: Add apply or exit option with green or red button

// TODO: Add game 3 game speeds 1000ms, 500 ms, 250ms
// TODO: Add a game mode single player, player vs player

// TODO: Refactor, put all global variables to a struct

static uint32_t lastProcessMs = 0;
static uint32_t currentLevelNum = LEVEL_1;
static const Level * currentLevel = {0};
static uint32_t currentLevelSeqIdx = 1;
static uint32_t levelIdx = 0;
static volatile Keys input = {0};
static uint32_t retries = 0;
static GameSettings settings = {0};

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
    case KEY_MENU: input.menu = isPressed; break;
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

static void stateInitEnter()
{
    lastProcessMs = HAL_GetTick();
    OLED_Init(&hi2c1);
    OLED_FillScreen(Black);
    OLED_SetCursor(0, 0);
    OLED_SetTextSize(FontSize24);
    OLED_Printf(" SIMON!");
    OLED_UpdateScreen();
    if (!gameSettingsRead(&settings)) {
        gameSettingsReset();
        gameSettingsRead(&settings);
    }
    keyscanInit(&onKeyPressCallback);
    notePlayerInit(&onPlaybackFinished);
    effectManagerInit(&onEffectFinished);
    notePlayerPlayMelody(getMelody(MelodyPowerOn), getMelodyLength(MelodyPowerOn));
    effectManagerPlayPowerOn();
}

static void stateInitProcess()
{
    // TODO: show cat wake up animation
}

static void stateIdleEnter()
{
    lastProcessMs = HAL_GetTick();
    effectManagerStopAllEffects();
    effectManagerPlayEffect(EFFECT_BREATHE, LED_GREEN, 0, 2000);
    OLED_FillScreen(Black);
    OLED_SetCursor(0, 0);
    OLED_SetTextSize(FontSize24);
    // TODO: show cat sleep animation
    OLED_Printf("  IDLE");
    OLED_UpdateScreen();
    input.state = 0; // reset input state
}

static void stateIdleProcess()
{
    if (input.green) {
        effectManagerPlayEffect(EFFECT_BLINK, LED_ALL, 300, 300 / 4);
        notePlayerPlayMelody(getMelody(MelodySuccess), getMelodyLength(MelodySuccess));
        gameStateProcessEvent(EVENT_INPUT_RECEIVED);
        retries = 0;
    }

    if (input.menu) {
        gameStateProcessEvent(EVENT_INPUT_MENU);
        return;
    }
}

static void oledShowSequence(const char stateStr[])
{
    OLED_FillScreen(Black);
    OLED_SetCursor(0, 0);
    OLED_SetTextSize(FontSize12);
    OLED_Printf("Lvl:%i%i/%i%i\n", (currentLevelNum + 1) / 10, (currentLevelNum + 1) % 10,
                LEVEL_COUNT / 10, LEVEL_COUNT % 10);
    OLED_Printf("%s:%i%i/%i%i", stateStr, levelIdx / 10, levelIdx % 10,
                currentLevelSeqIdx / 10, currentLevelSeqIdx % 10);
    OLED_UpdateScreen();
}

static void stateShowLevelEnter()
{
    lastProcessMs = HAL_GetTick();
    effectManagerStopAllEffects();
    levelIdx = 0;
    if (GAME_SEQUENCE_STATIC == settings.sequence) {
        currentLevel = levelsGetStaticLevel(currentLevelNum);
    } else if (GAME_SEQUENCE_RANDOM == settings.sequence) {
        levelsGenerateRandomLevel(currentLevelNum);
        currentLevel = levelsGetRandomLevel(currentLevelNum);
    }
    oledShowSequence("Showing");
}

static void stateShowLevelExit()
{
    // TODO: maybe confirm readiness for input with a melody
    effectManagerStopAllEffects();
    // notePlayerPlayMelody(getMelody(MelodyConfirm), getMelodyLength(MelodyConfirm));
    levelIdx = 0;
}

static void stateShowLevelProcess()
{
    if (input.menu) {
        gameStateProcessEvent(EVENT_INPUT_MENU);
        return;
    }

    if (input.state) {
        retries = 0;
        gameStateProcessEvent(EVENT_SEQUENCE_CANCELED);
        return;
    }

    if (effectManagerIsPlaying()) {
        return;
    }

    if (levelIdx >= currentLevelSeqIdx) {
        gameStateProcessEvent(EVENT_SEQUENCE_SHOWN);
    } else {
        // Show the level sequence
        Key key = currentLevel->sequence[levelIdx++];
        oledShowSequence("Showing");
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
    lastProcessMs = HAL_GetTick();
    oledShowSequence("Input");
}

static void stateUserInputProcess()
{
    if (input.menu) {
        gameStateProcessEvent(EVENT_INPUT_MENU);
        return;
    }

    if (isTimeoutHappened(USER_INPUT_TIMEOUT_MS)) {
        retries++;
        if (retries >= LEVEL_SHOW_RETRIES) {
            gameStateProcessEvent(EVENT_STATE_TIMEOUT);
            // TODO: add short idle sound
        } else {
            gameStateProcessEvent(EVENT_INPUT_TIMEOUT);
        }
        return;
    }

    if (0 == input.state) {
        return;
    }

    lastProcessMs = HAL_GetTick();
    retries = 0;

    // Critical section to avoid concurrent access to the input variable
    keyscanDisableIrq();
    Keys userInput = input; // copy state, should be atomic operation
    input.state &= ~userInput.state; // reset input state bits
    keyscanEnableIrq();
    const Key levelKey = currentLevel->sequence[levelIdx];
    if (userInput.state & (1 << levelKey)) {
        levelIdx++;
        oledShowSequence("Input");
        Note n = keyNoteMap[levelKey];
        notePlayerPlayNote(n.note, n.duration);
        effectManagerPlayEffect(EFFECT_FAST_RUMP, keyLedMap[levelKey], n.duration, n.duration);
        if (levelIdx >= currentLevelSeqIdx) {
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
    currentLevelSeqIdx++;
    if (currentLevelSeqIdx > currentLevel->keyCount) {
        currentLevelNum = (currentLevelNum + 1) % LEVEL_COUNT;
        if (GAME_SEQUENCE_STATIC == settings.sequence) {
            currentLevel = levelsGetStaticLevel(currentLevelNum);
        } else if (GAME_SEQUENCE_RANDOM == settings.sequence) {
            levelsGenerateRandomLevel(currentLevelNum);
            currentLevel = levelsGetRandomLevel(currentLevelNum);
        }
        currentLevelSeqIdx = 1;
        gameSettingsRead(&settings);
        settings.level = currentLevelNum;
        gameSettingsWrite(&settings);
    }
    levelIdx = 0;

    // TODO: show happy cat
    OLED_FillScreen(Black);
    OLED_SetCursor(0, 0);
    OLED_SetTextSize(FontSize24);
    OLED_Printf("SUCCESS");
    OLED_UpdateScreen();
}

static void stateSuccessProcess()
{
    // TODO: maybe process animation
}

static void stateFailEnter()
{
    lastProcessMs = HAL_GetTick();
    effectManagerStopAllEffects();
    effectManagerPlayEffect(EFFECT_BLINK, LED_RED, 500, 500 / 4);
    notePlayerPlayMelody(getMelody(MelodyFail), getMelodyLength(MelodyFail));
    // TODO: show sad cat
    OLED_FillScreen(Black);
    OLED_SetCursor(0, 0);
    OLED_SetTextSize(FontSize24);
    OLED_Printf(" FAILED");
    OLED_UpdateScreen();
}

static void stateFailProcess()
{
    // TODO: maybe process animation
}

static void statePowerOffEnter()
{
    lastProcessMs = HAL_GetTick();
    OLED_FillScreen(Black);
    OLED_SetCursor(0, 0);
    OLED_SetTextSize(FontSize24);
    OLED_Printf(" SLEEP");
    OLED_UpdateScreen();
    // OLED_FillScreen(Black);
    // OLED_UpdateScreen();
    // OLED_DisplayOff();
    effectManagerPlayPowerOff();
    // TODO: power off animation and sound
    // Shut down MCU
}

static void statePowerOffProcess()
{
    // TODO: proper wake up from power off
    if (input.state) {
        gameStateProcessEvent(EVENT_INPUT_RECEIVED);
    }
}

static void stateMenuEnter()
{
    lastProcessMs = HAL_GetTick();
    input.state = 0;
    gameMenuInit();
}

static void stateMenuExit()
{
    gameSettingsRead(&settings);
}

static void stateMenuProcess()
{
    if (0 == input.state) {
        gameMenuProcess();
        return;
    }

    lastProcessMs = HAL_GetTick();
    // Critical section to avoid concurrent access to the input variable
    keyscanDisableIrq();
    Keys userInput = input; // copy state, should be atomic operation
    input.state &= ~userInput.state; // reset input state bits
    keyscanEnableIrq();

    if (userInput.blue) {
        gameMenuProcessAction(MENU_ACTION_UP);
    } else if (userInput.yellow) {
        gameMenuProcessAction(MENU_ACTION_DOWN);
    } else if (userInput.green) {
        gameMenuProcessAction(MENU_ACTION_SELECT);
    } else if (userInput.red) {
        gameMenuProcessAction(MENU_ACTION_BACK);
    } else if (userInput.menu) {
        gameMenuProcessAction(MENU_ACTION_MENU);
    }
}

static void processStateTimeout()
{
    uint32_t stateTimeoutMs = gameStateGetTimeout();
    if (stateTimeoutMs && isTimeoutHappened(stateTimeoutMs)) {
        gameStateProcessEvent(EVENT_STATE_TIMEOUT);
    }
}

void gameInit()
{
    // State definition table
    static const GameStateDef stateDefs[] = {
        [GAME_STATE_NONE]          = {NULL, NULL, NULL, 0},
        [GAME_STATE_INIT]          = {stateInitEnter, NULL , stateInitProcess, INIT_STATE_TIMEOUT_MS},
        [GAME_STATE_IDLE]          = {stateIdleEnter, NULL, stateIdleProcess, POWER_OFF_TIMEOUT_MS},
        [GAME_STATE_SHOWING_LEVEL] = {stateShowLevelEnter, stateShowLevelExit, stateShowLevelProcess, 0},
        [GAME_STATE_USER_INPUT]    = {stateUserInputEnter, NULL, stateUserInputProcess, 0},
        [GAME_STATE_SUCCESS]       = {stateSuccessEnter, stateSuccessProcess, NULL, SUCCESS_STATE_TIMEOUT_MS},
        [GAME_STATE_FAILED]        = {stateFailEnter, stateFailProcess, NULL, FAILURE_STATE_TIMEOUT_MS},
        [GAME_STATE_OFF]           = {statePowerOffEnter, NULL, statePowerOffProcess, 0},
        [GAME_STATE_MENU]          = {stateMenuEnter, stateMenuExit, stateMenuProcess, MENU_TIMEOUT_MS},
    };
    gameStateInit(stateDefs);
    gameStateProcessEvent(EVENT_START);
}

void gameProcess()
{
    gameStateProcess();
    processStateTimeout();
    effectManagerProcess();
}
