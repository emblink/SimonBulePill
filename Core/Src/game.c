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

// Time constants
#define SECOND 1000
#define MINUTE (60 * SECOND)

// Timeouts and intervals
#define INPUT_TIMEOUT_MS (10 * SECOND)
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

typedef enum {
    GAME_STATE_INIT,          // Startup animation and sound
    GAME_STATE_IDLE,          // Idle animation
    GAME_STATE_SHOWING_LEVEL, // Display current level (lights/sounds)
    GAME_STATE_USER_INPUT,    // User input sequence verification
    GAME_STATE_SUCCESS,       // Success feedback (animation, sound)
    GAME_STATE_FAILED,        // Failure feedback (animation, sound)
    GAME_STATE_OFF,
    GAME_STATE_COUNT
} GameState;

typedef enum {
    EVENT_START,
    EVENT_STATE_TIMEOUT,
    EVENT_SEQUENCE_SHOWN,
    EVENT_INPUT_RECEIVED,
    EVENT_INPUT_TIMEOUT,
    EVENT_INPUT_CORRECT,
    EVENT_INPUT_WRONG,
    EVENT_COUNT
} Event;

// State definitions
typedef struct GameStateDef {
    void (*onEnter)(void);
    void (*onExit)(void);
    void (*onProcess)(void);
    uint32_t stateProcessTimeoutMs;
} GameStateDef;

// State transitions
typedef struct {
    GameState nextState;
    void (*action)(void);
    uint32_t delayMs;
} StateTransition;

// TODO: Add game 3 game speeds 1000ms, 500 ms, 250ms
// TODO: Add storage on flash, store current level, speed, game mode, etc
// TODO: Add reset to defaults
// TODO: Add random level mode
// TODO: Add input timeout after second show level run

static uint32_t lastProcessMs = 0;
static uint32_t nextProcessMs = 0;
static uint32_t currentLevel = 1;
static uint32_t levelIdx = 0;
static GameState volatile gameState = GAME_STATE_INIT;
static volatile bool effectFinished = false;
static volatile union {
	struct {
		uint8_t red : 1;
		uint8_t green : 1;
		uint8_t blue : 1;
		uint8_t yellow : 1;
	};
	uint32_t state;
} Keys = {0};

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
	effectFinished = true;
}

static void onKeyPressCallback(Key key, bool isPressed)
{
	switch (key) {
	case KEY_RED: Keys.red = isPressed; break;
	case KEY_GREEN: Keys.green = isPressed; break;
	case KEY_BLUE: Keys.blue = isPressed; break;
	case KEY_YELLOW: Keys.yellow = isPressed; break;
	default: break;
	}

    processInputEvent(key, isPressed);
}

static bool canProcess()
{
	uint32_t tick = HAL_GetTick();
	uint32_t diff = tick - lastProcessMs;
	bool res = diff >= nextProcessMs;
	return res;
}

static void processIdle()
{
	if (!effectManagerIsPlaying()) {
		effectManagerPlayEffect(EFFECT_BREATHE, LED_GREEN, 0, 2000);
	}

	if (Keys.green) {
		effectManagerStopAllEffects();
		effectManagerPlayEffect(EFFECT_BLINK, LED_ALL, 300, 300 / 3);
		notePlayerPlayMelody(getMelody(MelodySuccess), getMelodyLength(MelodySuccess));
		lastProcessMs = HAL_GetTick();
		nextProcessMs = 750;
		gameState = GAME_STATE_SHOWING_LEVEL;
		Keys.state = 0; // reset current keys state before show level state
	}
}

static void processShowLevel()
{
    // Skip level show if some key was pressed
    if (Keys.state) {
        // Confirm readiness for input with a melody
    	effectManagerStopAllEffects();
        notePlayerPlayMelody(getMelody(MelodyConfirm), getMelodyLength(MelodyConfirm));
//        effectManagerPlayEffect(EFFECT_FAST_RUMP, LED_ALL, 250, 250);
        levelIdx = 0;
        gameState = GAME_STATE_READING_INPUTS;
    } else if (canProcess()) {
        // Show the level sequence
        if (levelIdx >= currentLevel) {
        	levelIdx = 0;
        	gameState = GAME_STATE_READING_INPUTS;
        } else {
        	lastProcessMs = HAL_GetTick();
            Key key = levels[levelIdx++];
            Note n = keyNoteMap[key];
            notePlayerPlayNote(n.note, n.duration);
            effectManagerPlayEffect(EFFECT_FAST_RUMP, keyLedMap[key], n.duration, n.duration);
            nextProcessMs = n.duration + 250;
        }
    }
}

static void processSuccess()
{
	lastProcessMs = HAL_GetTick();
	effectManagerStopAllEffects();
	effectManagerPlayEffect(EFFECT_BLINK, LED_ALL, 300, 300 / 3);
	notePlayerPlayMelody(getMelody(MelodySuccess), getMelodyLength(MelodySuccess));
	currentLevel++;
	if (currentLevel >= LEVELS_COUNT) {
		// TODO: indicate
		currentLevel = 1;
	}
	levelIdx = 0;
	nextProcessMs = 1000;
	Keys.state = 0; // reset current keys state before show level state
	gameState = GAME_STATE_SHOWING_LEVEL;
}

static void processFail()
{
	lastProcessMs = HAL_GetTick();
	effectManagerStopAllEffects();
	effectManagerPlayEffect(EFFECT_BLINK, LED_RED, 500, 500 / 4);
	notePlayerPlayMelody(getMelody(MelodyFail), getMelodyLength(MelodyFail));
	levelIdx = 0;
	nextProcessMs = 1000;
	Keys.state = 0; // reset current keys state before show level state
	gameState = GAME_STATE_SHOWING_LEVEL;
}

static void processInputTimeout()
{
	// If input timeout reached, indicate and switch to the show level state
	uint32_t tick = HAL_GetTick();
	if (tick - lastProcessMs >= INPUT_TIMEOUT_MS) {
		lastProcessMs = HAL_GetTick();
		effectManagerStopAllEffects();
		effectManagerPlayEffect(EFFECT_BLINK, LED_RED, 500, 500 / 3);
		notePlayerPlayMelody(getMelody(MelodyFail), getMelodyLength(MelodyFail));
		gameState = GAME_STATE_IDLE;
	}
}

void oledUpdate()
{
	if (GAME_STATE_OFF == gameState) {
		OLED_DisplayOff();
		return;
	}

	OLED_FillScreen(Black);
	OLED_SetCursor(0, 0);
	if (GAME_STATE_IDLE == gameState) {
		// TODO: show cat sleep animation
		OLED_SetTextSize(FontSize24);
		OLED_Printf("  IDLE");
	} else if (GAME_STATE_READING_INPUTS == gameState) {
		OLED_SetTextSize(FontSize12);
		OLED_Printf("Lvl:%i%i/%i%i\n", currentLevel / 10, currentLevel % 10,
					LEVELS_COUNT/ 10, LEVELS_COUNT % 10);
		OLED_Printf("Input:%i%i/%i%i", levelIdx / 10, levelIdx % 10,
					currentLevel / 10, currentLevel % 10);
	} else if (GAME_STATE_SHOWING_LEVEL == gameState) {
		OLED_SetTextSize(FontSize12);
		OLED_Printf("Lvl:%i%i/%i%i\n", currentLevel / 10, currentLevel % 10,
					LEVELS_COUNT/ 10, LEVELS_COUNT % 10);
		OLED_Printf("Showing:%i%i/%i%i", levelIdx / 10, levelIdx % 10,
					currentLevel / 10, currentLevel % 10);
	} else if (GAME_STATE_SUCCESS == gameState) {
		// TODO: show happy cat
		OLED_SetTextSize(FontSize24);
		OLED_Printf("  SUCCESS :)");
	} else if (GAME_STATE_FAILED == gameState) {
		OLED_SetTextSize(FontSize24);
		OLED_Printf("  FAIL :(");
		// TODO: show sad cat
	}

//		OLED_SetTextSize(FontSize12);
//		OLED_SetCursor(0, 16);
//		OLED_Printf("Mode:Static");

	OLED_UpdateScreen();
}

static void stateInitEnter()
{
    OLED_Init(&hi2c1);
    OLED_FillScreen(Black);
    OLED_SetTextSize(FontSize24);
    OLED_Printf(" SIMON!");
    OLED_UpdateScreen();
    keyscanInit(&onKeyPressCallback);
    notePlayerInit(&onPlaybackFinished);
    effectManagerInit(&onEffectFinished);
    notePlayerPlayMelody(getMelody(MelodyPowerOn), getMelodyLength(MelodyPowerOn));
	effectManagerPlayPowerOn();
	gameState = GAME_STATE_IDLE;
}

static void stateInitProcess()
{

}

static void stateIdleEnter()
{

}

static void stateIdleProcess()
{

}

static void stateShowLevelEnter()
{

}

static void stateShowLevelProcess()
{

}

static void stateUserInputEnter()
{

}

static void stateUserInputProcess()
{

}

static void stateSuccessEnter()
{

}

static void stateFailEnter()
{

}

static void statePowerOffEnter()
{

}

static processEventSequenceCanceled()
{

}

// State definition table
static const GameStateDef stateDefs[] = {
    [GAME_STATE_INIT]          = {stateInitEnter, NULL, NULL, INIT_STATE_TIMEOUT_MS},
    [GAME_STATE_IDLE]          = {stateIdleEnter, NULL, stateIdleProcess, POWER_OFF_TIMEOUT_MS},
    [GAME_STATE_SHOWING_LEVEL] = {stateShowLevelEnter, NULL, stateShowLevelProcess, 0},
    [GAME_STATE_USER_INPUT]    = {stateUserInputEnter, NULL, stateUserInputProcess, INPUT_TIMEOUT_MS},
    [GAME_STATE_SUCCESS]       = {stateSuccessEnter, NULL, NULL, SUCCESS_STATE_TIMEOUT_MS},
    [GAME_STATE_FAILED]        = {stateFailEnter, NULL, NULL, FAILURE_STATE_TIMEOUT_MS},
    [GAME_STATE_OFF]           = {statePowerOffEnter, NULL, NULL, 0},
};

// State transition table
static const StateTransition stateTransitionTable[GAME_STATE_COUNT][EVENT_COUNT] = {
    [GAME_STATE_INIT] = {
        [EVENT_START] = { GAME_STATE_SHOWING_LEVEL, NULL, 0 },
    },
    [GAME_STATE_IDLE] = {
        [EVENT_INPUT_RECEIVED] = { GAME_STATE_SHOWING_LEVEL, NULL, 300 },
        [EVENT_STATE_TIMEOUT] = { GAME_STATE_OFF, NULL, 500 },
    },
    [GAME_STATE_SHOWING_LEVEL] = {
        [EVENT_SEQUENCE_SHOWN] = { GAME_STATE_USER_INPUT, NULL, 0 },
        [EVENT_SEQUENCE_CANCELED] = { GAME_STATE_USER_INPUT, NULL, 0 },
    },
    [GAME_STATE_USER_INPUT] = {
        [EVENT_INPUT_TIMEOUT] = { GAME_STATE_IDLE, NULL, 0 },
        [EVENT_INPUT_CORRECT] = { GAME_STATE_SUCCESS, NULL, 0 },
        [EVENT_INPUT_WRONG] = { GAME_STATE_FAILED, NULL, 0 },
    },
    [GAME_STATE_SUCCESS] = {
        [EVENT_STATE_TIMEOUT] = { GAME_STATE_SHOWING_LEVEL, NULL, SUCCESS_STATE_TIMEOUT_MS },
    },
    [GAME_STATE_FAILED] = {
        [EVENT_STATE_TIMEOUT] = { GAME_STATE_SHOWING_LEVEL, NULL, FAILURE_STATE_TIMEOUT_MS },
    },
    [GAME_STATE_OFF] = {},
};

static void changeState(GameState state, uint32_t delay)
{
	gameState = state;
	lastProcessMs = HAL_GetTick();
	nextProcessMs = 0;
}

static void processEvent()
{

}

void processUserInput(Key key, bool isPressed)
{
    if (!isPressed) {
        return;
    }

    if (GAME_STATE_USER_INPUT == gameState)
    {
        Key levelKey = levels[levelIdx];
        if (levelKey == key) {
            levelIdx++;
            Note n = keyNoteMap[key];
            notePlayerPlayNote(n.note, n.duration);
            effectManagerPlayEffect(EFFECT_FAST_RUMP, keyLedMap[key], n.duration, n.duration);
            if (levelIdx >= currentLevel) {
                gameState = GAME_STATE_SUCCESS;
                nextProcessMs = 1000;
            }
        } else {
            changeState(GAME_STATE_FAILED, 0);
            gameState = GAME_STATE_FAILED;
            nextProcessMs = 1000;
        }
    } else if (GAME_STATE_SHOWING_LEVEL == gameState) {
        processEventSequenceCanceled();
    }
}

static void processState()
{
    switch (gameState) {
	case GAME_STATE_INIT: processInit(); break;
	case GAME_STATE_IDLE: processIdle(); break;
	case GAME_STATE_SHOWING_LEVEL: processShowLevel(); break;
	case GAME_STATE_USER_INPUTS: processInputTimeout(); break;
	case GAME_STATE_SUCCESS: processSuccess(); break;
	case GAME_STATE_FAILED: processFail(); break;

	default: break;
	}
	effectManagerProcess();
	oledUpdate();
}

void gameInit()
{
    changeState(GAME_STATE_INIT, 0);
}

void gameProcess()
{
    processState();
}
