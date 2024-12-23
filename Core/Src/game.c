#include "game.h"
#include "notePlayer.h"
#include "effectManager.h"
#include "melodies.h"
#include "keyscan.h"
#include "levels.h"
#include "tim.h"

typedef enum {
	GAME_STATE_INIT, // Startup animation and sound
	GAME_STATE_IDLE, // Idle animation
    GAME_STATE_SHOWING_LEVEL, // Display current level (lights/sounds)
    GAME_STATE_READING_INPUTS, // User input sequence verification
	GAME_STATE_SUCCESS, // Success feedback (animation, sound)
	GAME_STATE_FAILED, // Failure feedback (animation, sound)
	GAME_STATE_OFF,
	GAME_STATE_COUNT
} GameState;

static volatile int buttonPwm[4] = {0};
static volatile int buttonInc[4] = {0};
static uint32_t lastProcessMs = 0;
static uint32_t nextProcessMs = 0;
static Melody melody = MelodyPowerOn;
static uint32_t currentLevel = 3;
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
	[KEY_RED] = { NOTE_C4, 500 },
	[KEY_GREEN] = { NOTE_E4, 500 },
	[KEY_BLUE] = { NOTE_G4, 500 },
	[KEY_YELLOW] = { NOTE_B4, 500 },
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
	if (!isPressed)
	{
		return;
	}

	switch (key) {
	case KEY_RED: Keys.red = 1; break;
	case KEY_GREEN: Keys.green = 1; break;
	case KEY_BLUE: Keys.blue = 1; break;
	case KEY_YELLOW: Keys.yellow = 1; break;
	default: break;
	}
}

static bool canProcess()
{
	uint32_t tick = HAL_GetTick();
	uint32_t diff = tick - lastProcessMs;
	bool res = diff >= nextProcessMs;
}

static void processInit()
{
	notePlayerPlayMelody(getMelody(MelodyPowerOn), getMelodyLength(MelodyPowerOn));
	effectManagerPlayPowerOn();
	gameState++;
	effectManagerPlayEffect(EFFECT_BREATHE, LED_GREEN, 0);
}

static void processIdle()
{
	if (Keys.green) {
		Keys.state = 0;
		effectManagerStopAllEffects();
		effectManagerPlayEffect(EFFECT_FAST_BLINK, LED_ALL, 600);
		notePlayerPlayMelody(getMelody(MelodySuccess), getMelodyLength(MelodySuccess));
		lastProcessMs = HAL_GetTick();
		nextProcessMs = 1000;
		gameState++;
	}
}

static void processShowLevel()
{
	if (!canProcess()) {
		return;
	}

	lastProcessMs = HAL_GetTick();
	if (levelIdx < currentLevel) {
		Key key = levels[levelIdx++];
		Note n = keyNoteMap[key];
		notePlayerPlayNote(n.note, n.duration);
		effectManagerPlayEffect(EFFECT_FAST_BREATHE, keyLedMap[key], n.duration);
		nextProcessMs = n.duration + 200;
	} else {
		gameState++;
		nextProcessMs = 0;
	}
}

static void processInput()
{

}

static void processSuccess()
{

}

static void processFail()
{

}

void gameInit()
{
	keyscanInit(&onKeyPressCallback);
	notePlayerInit(&onPlaybackFinished);
	effectManagerInit(&onEffectFinished);
}

void gameProcess()
{
	switch (gameState) {
	case GAME_STATE_INIT: processInit(); break;
	case GAME_STATE_IDLE: processIdle(); break;
	case GAME_STATE_SHOWING_LEVEL: processShowLevel(); break;
	case GAME_STATE_READING_INPUTS: processInput(); break;
	case GAME_STATE_SUCCESS: processSuccess(); break;
	case GAME_STATE_FAILED: processFail(); break;

	default: break;
	}
	effectManagerProcess();
}
