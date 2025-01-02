#include "audioAmplifier.h"
#include "main.h"

static bool isShutdown = true;
static bool isMuted= true;

void audioAmplifierInit()
{
    audioAmplifierShutdown(false);
    audioAmplifierMute(true);
}

void audioAmplifierShutdown(bool shutdown)
{
    isShutdown = shutdown;
    HAL_GPIO_WritePin(AMP_SHUTDOWN_GPIO_Port, AMP_SHUTDOWN_Pin, shutdown ? 0 : 1);
}

void audioAmplifierMute(bool mute)
{
    isMuted = mute;
    HAL_GPIO_WritePin(AMP_MUTE_GPIO_Port, AMP_MUTE_Pin, mute ? 0 : 1);
}

bool audioAmplifierIsShutdown()
{
    return isShutdown;
}

bool audioAmplifierIsMuted()
{
    return isMuted;
}
