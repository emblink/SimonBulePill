#include "game.h"
#include "notePlayer.h"
#include "effectManager.h"
#include "melodies.h"
#include "keyscan.h"
#include "levels.h"
#include "tim.h"
#include "i2c.h"
#include "oled.h"

#define INPUT_TIMEOUT_MS 10000
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

static void processInput(Key key)
{
	lastProcessMs = HAL_GetTick();
	Key levelKey = levels[levelIdx];
	if (levelKey == key) {
		levelIdx++;
		Note n = keyNoteMap[key];
		notePlayerPlayNote(n.note, n.duration);
		effectManagerPlayEffect(EFFECT_FAST_RUMP, keyLedMap[key], n.duration, n.duration);
		if (levelIdx >= currentLevel) {
			gameState = GAME_STATE_SUCCESS;
		}
	} else {
		gameState = GAME_STATE_FAILED;
	}
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

	if (GAME_STATE_READING_INPUTS == gameState && isPressed)
	{
		processInput(key);
	}
}

static bool canProcess()
{
	uint32_t tick = HAL_GetTick();
	uint32_t diff = tick - lastProcessMs;
	bool res = diff >= nextProcessMs;
	return res;
}

static void processInit()
{
	notePlayerPlayMelody(getMelody(MelodyPowerOn), getMelodyLength(MelodyPowerOn));
	effectManagerPlayPowerOn();
	gameState = GAME_STATE_IDLE;
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
    	lastProcessMs = HAL_GetTick();
        Key key = levels[levelIdx++];
        Note n = keyNoteMap[key];
        notePlayerPlayNote(n.note, n.duration);
        effectManagerPlayEffect(EFFECT_FAST_RUMP, keyLedMap[key], n.duration, n.duration);
        nextProcessMs = n.duration + 250;
        if (levelIdx >= currentLevel) {
        	levelIdx = 0;
        	gameState = GAME_STATE_READING_INPUTS;
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
	nextProcessMs = 1500;
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
	nextProcessMs = 1500;
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

static void oledTest()
{
	/* Draw circles */
	OLED_DrawCircle(OLEDWIDTH / 2, OLEDHEIGHT / 2, 10, White);
	OLED_FillCircle(OLEDWIDTH / 2, OLEDHEIGHT / 2, 4, White);
	OLED_UpdateScreen();
	HAL_Delay(1000);
	/* Draw trapezoid */
	uint8_t y = 0;
	while (y++ < OLEDHEIGHT) {
		OLED_DrawLine(0, y, OLEDWIDTH - y, y, White);
	}
	OLED_UpdateScreen();
	HAL_Delay(1000);
	/* Draw rectangles */
	OLED_FillScreen(Black);
	OLED_FillRoundRect(10, OLEDHEIGHT / 4, OLEDWIDTH - 20, OLEDHEIGHT / 2,
			OLEDHEIGHT / 4, White);
	OLED_FillRoundRect(20, OLEDHEIGHT / 2 - OLEDHEIGHT / 8, OLEDWIDTH - 40,
			OLEDHEIGHT / 4, OLEDHEIGHT / 8, Black);
	OLED_FillRoundRect(30, OLEDHEIGHT / 2 - OLEDHEIGHT / 16, OLEDWIDTH - 60,
			OLEDHEIGHT / 8, OLEDHEIGHT / 16, White);
	OLED_UpdateScreen();
	HAL_Delay(1000);
	/* Draw chessboard */
	OLED_FillScreen(Black);
	OLED_Color_t color = White;
	for (int i = 0; i < OLEDHEIGHT / 4; i++) {
		for (int j = 0; j < OLEDWIDTH / 4; j++) {
			OLED_FillRect(4 * j, 4 * i, 4, 4, color);
			color = !color;
		}
		color = !color;
	}
	OLED_UpdateScreen();
	HAL_Delay(2000);
	/* Test edges */
	OLED_FillScreen(White);
	OLED_FillRect(0, 0, 5, 5, Black);
	OLED_FillRect(OLEDWIDTH - 5, 0, 5, 5, Black);
	OLED_FillRect(0, OLEDHEIGHT - 5, 5, 5, Black);
	OLED_FillRect(OLEDWIDTH - 5, 0, 5, 5, Black);
	OLED_FillRect(OLEDWIDTH - 5, OLEDHEIGHT - 5, 5, 5, Black);
	OLED_UpdateScreen();
	color = White;
	HAL_Delay(2000);
	/* Draw white noise */
	for (int a = 0; a < OLEDWIDTH; a++) {
		for (int b = 0; b < OLEDHEIGHT; b++) {
			OLED_DrawPixel(a, b, color);
			color = !color;
		}
		color = !color;
		OLED_UpdateScreen();
	}
	/* Draw numbers */
	color = White;
	for (int a = 0; a < OLEDWIDTH; a++) {
		OLED_Printf("%i ", a);
		OLED_UpdateScreen();
	}
	HAL_Delay(2000);
	/* Draw text */
	for (int i = 0; i < 5; i++) {
		OLED_FillScreen(Black);
		OLED_SetCursor(0, 0);
		OLED_SetTextSize(i);
		for (int j = 0; j < 256; j++) {
			OLED_Printf("%c", j);
			OLED_UpdateScreen();
		}
		HAL_Delay(500);
	}
	HAL_Delay(1000);
	/* Toggle display on/off */
	OLED_FillScreen(White);
	OLED_UpdateScreen();
	HAL_Delay(1000);
	OLED_DisplayOff();
	HAL_Delay(1000);
}

void gameInit()
{
	OLED_Init(&hi2c1);
	OLED_FillScreen(White);
	OLED_UpdateScreen();
//	oledTest();
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
	case GAME_STATE_READING_INPUTS: processInputTimeout(); break;
	case GAME_STATE_SUCCESS: processSuccess(); break;
	case GAME_STATE_FAILED: processFail(); break;

	default: break;
	}
	effectManagerProcess();
}
