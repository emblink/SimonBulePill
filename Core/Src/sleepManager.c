#include <stdint.h>
#include "sleepManager.h"
#include "notePlayer.h"
#include "audioAmplifier.h"
#include "stm32f1xx_hal.h"

#define APM_MUTE_TIMEOUT_MS 250

static uint32_t lastPlaybackMs = 0;

void sleepManagerInit()
{
    // Init RTC
}

void sleepManagerProcess()
{
    if (notePlayerIsPlaying()) {
        lastPlaybackMs = HAL_GetTick();
    } else {
        if (!audioAmplifierIsMuted() && HAL_GetTick() - lastPlaybackMs >= APM_MUTE_TIMEOUT_MS) {
            audioAmplifierMute(true);
        }
    }

    __WFI();  // Wait For Interrupt instruction
    // HAL_IWDG_Refresh(&hiwdg);
}

