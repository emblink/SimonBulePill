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
    // Added AO3401A P-Channel MOSFET for amp power control with a 47k resistor
    // from VBAT to the GATE and AMP_SHUTDOWN_Pin
    // GATE - LOW, amp enabled
    // GATE - Hi-Z, amp disabled

    isShutdown = shutdown;
    if (shutdown) {
        // Set pin to Hi-Z (Input mode) to let the 47k pull-up turn the MOSFET OFF
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = AMP_SHUTDOWN_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        HAL_GPIO_Init(AMP_SHUTDOWN_GPIO_Port, &GPIO_InitStruct);
    } else {
        // Set pin as Output LOW to enable the amp
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = AMP_SHUTDOWN_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(AMP_SHUTDOWN_GPIO_Port, &GPIO_InitStruct);
        HAL_GPIO_WritePin(AMP_SHUTDOWN_GPIO_Port, AMP_SHUTDOWN_Pin, GPIO_PIN_RESET);
    }
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
