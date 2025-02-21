#include "gameScenePowerOff.h"
#include "game.h"
#include "generic.h"
#include "fontSize.h"
#include "oled.h"
#include "effectManager.h"
#include "audioAmplifier.h"
#include "melodies.h"
#include "notePlayer.h"

void gameScenePowerOffEnter()
{
    OLED_FillScreen(Black);
    OLED_SetCursor(0, 0);
    OLED_SetTextSize(FontSize16);
    OLED_Printf("Zzz Zzz");
    OLED_UpdateScreen();
    effectManagerPlayPowerOff();
    notePlayerPlayMelody(getMelody(MelodyPowerOff), getMelodyLength(MelodyPowerOff));
}

void gameScenePowerOffProcess()
{
    if (effectManagerIsPlaying() || notePlayerIsPlaying()) {
        return;
    }

    OLED_DisplayOff();
    audioAmplifierMute(true);
    gameOnSceneEnd();
}