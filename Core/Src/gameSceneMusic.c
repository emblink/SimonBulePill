#include <stdlib.h>
#include "gameSceneMusic.h"
#include "melodies.h"
#include "notePlayer.h"
#include "effectManager.h"
#include "gameState.h"
#include "audioAmplifier.h"
#include "i2c.h"
#include "oled.h"
#include "fontSize.h"
#include "game.h"

static Melody melodiesToKeyMap[] = {
    [KEY_GREEN] = MelodyJingleBells,
    [KEY_BLUE] = MelodyHarryPotter,
    [KEY_RED] = MelodyStarWars,
    [KEY_YELLOW] = MelodyDvaVeselihGusya,
};

static void onMusicKey(Key key, bool pressed)
{
	if (!pressed) {
		return;
	}

    if (notePlayerIsPlaying()) {
        notePlayerStop();
    }
    notePlayerPlayMelody(getMelody(melodiesToKeyMap[key]), getMelodyLength(melodiesToKeyMap[key]));
}

static void onNoteStart(uint32_t noteHz, uint32_t duration)
{
    (void) noteHz;
    audioAmplifierMute(false);
    srand(noteHz); // same note will produce same random number
    Led led = rand() % LED_COUNT;
    effectManagerPlayEffect(EFFECT_FAST_RUMP, led, duration, duration);
}

void gameSceneMusicEnter()
{
    OLED_FillScreen(Black);
    OLED_SetCursor(0, 0);
    OLED_SetTextSize(FontSize16);
    OLED_Printf("  Music");
    OLED_UpdateScreen();
    gameSetSceneInputCb(onMusicKey);
    notePlayerInit(&onNoteStart, NULL);
}

void gameSceneMusicProcess()
{

}
