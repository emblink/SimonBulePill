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
#include "audioAmplifier.h"
#include "animationSystem.h"
#include "generic.h"
#include "sceneSimonStates.h"

// Timeouts and intervals
#define USER_INPUT_TIMEOUT_MS (10 * SECOND)
#define LEVEL_REPEAT_LIMIT 3
#define SUCCESS_STATE_TIMEOUT_MS (SECOND)
#define FAILURE_STATE_TIMEOUT_MS (SECOND)

static const Level * currentLevel = NULL;
static uint32_t currentLevelSeqIdx = 1;
static uint32_t levelIdx = 0;
static uint32_t levelRepeatCount = 0;
static GameSettings settings = {0};

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

static void stateInitEnter()
{
    OLED_FillScreen(Black);
    OLED_UpdateScreen();
    gameSettingsRead(&settings);
    animationSystemPlay(ANIM_HAPPY, true);
    currentLevel = NULL;
    currentLevelSeqIdx = 1;
    levelIdx = 0;
    levelRepeatCount = 0;
}

static void oledShowSequence(const char stateStr[])
{
    OLED_FillScreen(Black);
    OLED_SetCursor(0, 0);
    OLED_SetTextSize(FontSize12);
    OLED_Printf("Level:%i\n", settings.level + 1);
    OLED_SetTextSize(FontSize16);
    OLED_Printf("%s:%i%i/%i%i", stateStr, levelIdx / 10, levelIdx % 10,
                currentLevelSeqIdx / 10, currentLevelSeqIdx % 10);
    OLED_UpdateScreen();
}

static void onUserInputKey(Key key, bool isPressed)
{
    if (!isPressed) {
        return;
    }

    levelRepeatCount = 0;
    gameStateResetTimeout();

    const Key levelKey = currentLevel->sequence[levelIdx];
    if (levelKey == key) {
        levelIdx++;
        oledShowSequence("Input");
        Note n = keyNoteMap[levelKey];
        uint32_t duration = gameSpeedToDuration[settings.speed];
        notePlayerPlayNote(n.note, duration);
        effectManagerPlayEffect(EFFECT_FAST_RUMP, keyLedMap[levelKey], duration, duration);
        if (levelIdx >= currentLevelSeqIdx) {
            gameStateProcessEventWithDelay(EVENT_INPUT_CORRECT, duration);
        }
    } else {
        // wrong key was pressed
        gameStateProcessEvent(EVENT_INPUT_WRONG);
    }
}

static void onShowLevelKey(Key key, bool isPressed)
{
    if (isPressed) {
        levelRepeatCount = 0;
        gameStateProcessEvent(EVENT_SEQUENCE_CANCELED);
    }
}

static void stateShowLevelEnter()
{
	animationSystemStop();
    effectManagerStopAllEffects();
    levelIdx = 0;

    if (levelRepeatCount > LEVEL_REPEAT_LIMIT) {
        levelRepeatCount = 0;
        gameStateProcessEvent(EVENT_LEVEL_REPEATED_TOO_MANY_TIMES);
        return;
    }
    levelRepeatCount++;

    if (GAME_SEQUENCE_STATIC == settings.sequence) {
        currentLevel = levelsGetStaticLevel(settings.level);
    } else if (GAME_SEQUENCE_RANDOM == settings.sequence) {
        if (NULL == currentLevel) {
            levelsGenerateRandomLevel(settings.level);
            currentLevel = levelsGetRandomLevel(settings.level);
        }
    }
    oledShowSequence("Show");
    gameSetSceneInputCb(onShowLevelKey);
}

static void stateShowLevelExit()
{
    effectManagerStopAllEffects();
    gameSetSceneInputCb(NULL);
    levelIdx = 0;
}

static void stateShowLevelProcess()
{
    if (effectManagerIsPlaying()) {
        return;
    }

    if (levelIdx >= currentLevelSeqIdx) {
        gameStateProcessEvent(EVENT_SEQUENCE_SHOWN);
    } else {
        // Show the level sequence
        Key key = currentLevel->sequence[levelIdx++];
        oledShowSequence("Show");
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
    oledShowSequence("Input");
    gameSetSceneInputCb(onUserInputKey);
}

static void stateUserInputExit()
{
    gameSetSceneInputCb(NULL);
}

static void stateSuccessEnter()
{
    effectManagerStopAllEffects();
    gamePlaySuccessIndicaton();
    currentLevelSeqIdx++;
    if (currentLevelSeqIdx > currentLevel->keyCount) {
        settings.level = (settings.level + 1) % LEVEL_COUNT;
        gameSettingsWrite(&settings);
        if (GAME_SEQUENCE_STATIC == settings.sequence) {
            currentLevel = levelsGetStaticLevel(settings.level);
        } else if (GAME_SEQUENCE_RANDOM == settings.sequence) {
            levelsGenerateRandomLevel(settings.level);
            currentLevel = levelsGetRandomLevel(settings.level);
        }
        currentLevelSeqIdx = 1;
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
    gamePlayFailedIndication();
    animationSystemPlay(ANIM_SAD, true);
}

static void stateFailExit()
{
    animationSystemStop();
}

static void stateIdleEnter()
{
    effectManagerStopAllEffects();
    notePlayerStop();
    animationSystemStop();
    OLED_FillScreen(Black);
    gameOnSceneEnd();
}

void gameSceneSimonEnter()
{
    // State definition table
    static const GameStateDef stateDefs[] = {
        [GAME_STATE_INIT]          = {stateInitEnter, NULL, NULL, 500},
        [GAME_STATE_SHOWING_LEVEL] = {stateShowLevelEnter, stateShowLevelExit, stateShowLevelProcess, 0},
        [GAME_STATE_USER_INPUT]    = {stateUserInputEnter, stateUserInputExit, NULL, USER_INPUT_TIMEOUT_MS},
        [GAME_STATE_SUCCESS]       = {stateSuccessEnter, stateSuccessExit, NULL, SUCCESS_STATE_TIMEOUT_MS},
        [GAME_STATE_FAILED]        = {stateFailEnter, stateFailExit, NULL, FAILURE_STATE_TIMEOUT_MS},
        [GAME_STATE_IDLE]          = {stateIdleEnter, NULL, NULL, 0},
    };
    gameStateInit(stateDefs);
}

void gameSceneSimonProcess()
{
    gameStateProcess();
}
