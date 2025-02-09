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
#include "audioAmplifier.h"
#include "batteryManager.h"
#include "animationSystem.h"

// Time constants
#define SECOND 1000
#define MINUTE (60 * SECOND)

// Timeouts and intervals
#define USER_INPUT_TIMEOUT_MS (10 * SECOND)
#define LEVEL_REPEAT_LIMIT 3
#define POWER_OFF_TIMEOUT_MS (1 * MINUTE)
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

// Nessesary
// Fix fw stuck after standby wake up


// Optional
// TODO: Add a game mode single player, player vs player
// TODO: Refactor, put all global variables to a struct
// TODO: Add transition to Idle state sound
// TODO: Add power off animation

static uint32_t currentLevelNum = LEVEL_1;
static const Level * currentLevel = {0};
static uint32_t currentLevelSeqIdx = 1;
static uint32_t levelIdx = 0;
static volatile Keys input = {0};
static uint32_t levelRepeatCount = 0;
static GameSettings settings = {0};
static uint32_t batteryCharge = 0;
static uint32_t lastBatteryCheckMs = 0;

static const Note keyNoteMap[] = {
    [KEY_RED] = { NOTE_C4, 0 },
    [KEY_GREEN] = { NOTE_E4, 0 },
    [KEY_BLUE] = { NOTE_G4, 0 },
    [KEY_YELLOW] = { NOTE_C5, 0 },
};

static const Led keyLedMap[] = {
    [KEY_RED] = LED_RED,
    [KEY_GREEN] = LED_GREEN,
    [KEY_BLUE] = LED_BLUE,
    [KEY_YELLOW] = LED_YELLOW,
};

static const uint32_t gameSpeedToDuration[] = {
    [GAME_SPEED_SLOW] = 800,
    [GAME_SPEED_MEDIUM] = 500,
    [GAME_SPEED_HIGH] = 250,
};

static void batteryCheck()
{
    batteryCharge = batteryManagerGetPercent();
}

static void batteryProcess()
{
    uint32_t currentTick = HAL_GetTick();
    if (currentTick - lastBatteryCheckMs > 5 * MINUTE) {
        lastBatteryCheckMs = currentTick;
        batteryCheck();
    }

    if (0 == batteryCharge) {
        gameStateProcessEvent(EVENT_BATTERY_LOW);
    }
}

static void showBatteryCharge()
{
    OLED_SetCursor(105, 0);
    OLED_SetTextSize(FontSize12);
    OLED_Printf("%i", batteryCharge);
}

static inline void ledOn()
{
//	HAL_GPIO_WritePin(LED_BOARD_GPIO_Port, LED_BOARD_Pin, 0);
}

static inline void ledOff()
{
//	HAL_GPIO_WritePin(LED_BOARD_GPIO_Port, LED_BOARD_Pin, 1);
}

static void onPlaybackFinished()
{
	// audioAmplifierMute(true); // makes bumping sound in the end even if the sound sine fades properly
	ledOff();
}

static void onPlaybackStarted()
{
	audioAmplifierMute(false);
	ledOn();
}

static void onEffectFinished(Led led)
{
    if (GAME_STATE_INIT == gameStateGetCurrentState())
    {
        gameStateProcessEvent(EVENT_INITED);
    }
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

static void stateInitEnter()
{
    batteryCheck();
    audioAmplifierInit();
    OLED_Init(&hi2c1);
    OLED_FillScreen(Black);
    OLED_UpdateScreen();
    if (!gameSettingsRead(&settings)) {
        gameSettingsReset();
        gameSettingsRead(&settings);
    }
    keyscanInit(&onKeyPressCallback);
    notePlayerInit(&onPlaybackStarted, &onPlaybackFinished);
    effectManagerInit(&onEffectFinished);
    notePlayerPlayMelody(getMelody(MelodyPowerOn), getMelodyLength(MelodyPowerOn));
    effectManagerPlayPowerOn();
    animationSystemPlay(ANIM_HELLO, true);
}

static void stateInitExit()
{
    animationSystemStop();
}

static void stateIdleEnter()
{
    effectManagerStopAllEffects();
    animationSystemPlay(ANIM_SLEEP, true);
    input.state = 0; // reset input state
}

static void stateIdleExit()
{
	animationSystemStop();
}

static void stateIdleProcess()
{
    if (input.green || input.red || input.blue || input.yellow) {
        effectManagerPlayEffect(EFFECT_BLINK, LED_ALL, 300, 300 / 4);
        notePlayerPlayMelody(getMelody(MelodySuccess), getMelodyLength(MelodySuccess));
        gameStateProcessEvent(EVENT_INPUT_RECEIVED);
        levelRepeatCount = 0;
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
    showBatteryCharge();
    OLED_UpdateScreen();
}

static void stateShowLevelEnter()
{
    effectManagerStopAllEffects();
    levelIdx = 0;

    levelRepeatCount++;
    if (levelRepeatCount > LEVEL_REPEAT_LIMIT) {
        levelRepeatCount = 0;
        gameStateProcessEvent(EVENT_LEVEL_REPEATED_TOO_MANY_TIMES);
        return;
    }

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
    effectManagerStopAllEffects();
    levelIdx = 0;
}

static void stateShowLevelProcess()
{
    if (input.menu) {
        gameStateProcessEvent(EVENT_INPUT_MENU);
        return;
    }

    if (input.state) {
        levelRepeatCount = 0;
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
        uint32_t duration = gameSpeedToDuration[settings.speed];
        notePlayerPlayNote(n.note, duration);
        effectManagerPlayEffect(EFFECT_FAST_RUMP, keyLedMap[key], duration, duration);
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
    if (input.menu) {
        gameStateProcessEvent(EVENT_INPUT_MENU);
        return;
    }

    if (0 == input.state) {
        return;
    }

    levelRepeatCount = 0;

    // Critical section to avoid concurrent access to the input variable
    keyscanDisableIrq();
    Keys userInput = input; // copy state, should be atomic operation
    input.state &= ~userInput.state; // reset input state bits
    keyscanEnableIrq();
    const Key levelKey = currentLevel->sequence[levelIdx];
    if (userInput.state & (1 << levelKey)) {
        gameStateResetTimeout();
        levelIdx++;
        oledShowSequence("Input");
        Note n = keyNoteMap[levelKey];
        uint32_t duration = gameSpeedToDuration[settings.speed];
        notePlayerPlayNote(n.note, duration);
        effectManagerPlayEffect(EFFECT_FAST_RUMP, keyLedMap[levelKey], duration, duration);
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

    animationSystemPlay(ANIM_HAPPY, true);
}

static void stateSuccessExit()
{
    animationSystemStop();
}

static void stateFailEnter()
{
    effectManagerStopAllEffects();
    effectManagerPlayEffect(EFFECT_BLINK, LED_RED, 500, 500 / 4);
    notePlayerPlayMelody(getMelody(MelodyFail), getMelodyLength(MelodyFail));
    animationSystemPlay(ANIM_SAD, true);
}

static void stateFailExit()
{
    animationSystemStop();
}

static void statePowerOffEnter()
{
    OLED_FillScreen(Black);
    OLED_SetCursor(0, 0);
    OLED_SetTextSize(FontSize16);
    OLED_Printf(" SLEEP");
    OLED_UpdateScreen();
    HAL_Delay(1000);
    OLED_DisplayOff();
    effectManagerPlayPowerOff();
    // add power off animation and sound
    // Shut down MCU
}

static void statePowerOffProcess()
{
    // add proper wake up from power off
    if (input.state) {
        gameStateProcessEvent(EVENT_INPUT_RECEIVED);
    }
}

static void stateMenuEnter()
{
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

    // Critical section to avoid concurrent access to the input variable
    keyscanDisableIrq();
    Keys userInput = input; // copy state, should be atomic operation
    input.state &= ~userInput.state; // reset input state bits
    keyscanEnableIrq();
    gameStateResetTimeout();

    if (userInput.blue) {
        gameMenuProcessAction(MENU_ACTION_UP);
    } else if (userInput.yellow) {
        gameMenuProcessAction(MENU_ACTION_DOWN);
    } else if (userInput.green) {
        gameMenuProcessAction(MENU_ACTION_SELECT);
    } else if (userInput.menu) {
        gameMenuProcessAction(MENU_ACTION_MENU);
    }
}

void gameInit()
{
    // State definition table
    static const GameStateDef stateDefs[] = {
        [GAME_STATE_NONE]          = {NULL, NULL, NULL, 0},
        [GAME_STATE_INIT]          = {stateInitEnter, stateInitExit, NULL, 0},
        [GAME_STATE_IDLE]          = {stateIdleEnter, stateIdleExit, stateIdleProcess, POWER_OFF_TIMEOUT_MS},
        [GAME_STATE_SHOWING_LEVEL] = {stateShowLevelEnter, stateShowLevelExit, stateShowLevelProcess, 0},
        [GAME_STATE_USER_INPUT]    = {stateUserInputEnter, NULL, stateUserInputProcess, USER_INPUT_TIMEOUT_MS},
        [GAME_STATE_SUCCESS]       = {stateSuccessEnter, stateSuccessExit, NULL, SUCCESS_STATE_TIMEOUT_MS},
        [GAME_STATE_FAILED]        = {stateFailEnter, stateFailExit, NULL, FAILURE_STATE_TIMEOUT_MS},
        [GAME_STATE_OFF]           = {statePowerOffEnter, NULL, statePowerOffProcess, 0},
        [GAME_STATE_MENU]          = {stateMenuEnter, stateMenuExit, stateMenuProcess, MENU_TIMEOUT_MS},
    };
    gameStateInit(stateDefs);
    gameStateProcessEvent(EVENT_START);
}

void gameProcess()
{
    gameStateProcess();
    effectManagerProcess();
    animationSystemProcess();
    batteryProcess();
}
