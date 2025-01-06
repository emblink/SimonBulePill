#include <stdint.h>
#include "sleepManager.h"
#include "notePlayer.h"
#include "audioAmplifier.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_rtc.h"
#include "rtc.h"
#include "main.h"
#include "effectManager.h"
#include "keyscan.h"

#define APM_MUTE_TIMEOUT_MS 250

static uint32_t lastPlaybackMs = 0;
static volatile bool rtcWakeupFlag = false;

static inline void ledOn()
{
	HAL_GPIO_WritePin(LED_BOARD_GPIO_Port, LED_BOARD_Pin, 1);
}

static inline void ledOff()
{
	HAL_GPIO_WritePin(LED_BOARD_GPIO_Port, LED_BOARD_Pin, 0);
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
    rtcWakeupFlag = true;
}

void HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc)
{
	__asm volatile ("nop");
}

void sleepManagerInit()
{
    // Init RTC
    ledOn();
    RTC_DateTypeDef  sDate = {0};
    RTC_TimeTypeDef sTime = {0};
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
}

static void rtcAlarmEnable(uint32_t sleepDurationMs)
{
    // RTC clock is 40 Khz, prescaler is set to 40, so second counter ticks each milisecond
    RTC_AlarmTypeDef sAlarm = {0};
    sAlarm.AlarmTime.Hours = sleepDurationMs / 3600;
    sAlarm.AlarmTime.Minutes = (sleepDurationMs / 60) % 60;
    sAlarm.AlarmTime.Seconds = sleepDurationMs % 60;
    HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN);
}

static void rtcAlarmDisable()
{
    HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
}

static void enterStopMode()
{
    HAL_SuspendTick();
    // Reset RTC time to zero
    RTC_TimeTypeDef sTime = {0};
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    uint32_t sleepDurationMs = 0;
//    rtcAlarmEnable(); // Not used for now
    ledOff();
    // Enter Stop Mode
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
    // Exit Stop Mode
    ledOn();

    // Update SysTick counter after sleep wake up
    RTC_TimeTypeDef wakeUpTime = {0};
    HAL_RTC_GetTime(&hrtc, &wakeUpTime, RTC_FORMAT_BIN);
    uint32_t sleepMs = wakeUpTime.Hours * 3600 + wakeUpTime.Minutes * 60 + wakeUpTime.Seconds;
    extern __IO uint32_t uwTick;
    if (sleepMs < sleepDurationMs) {
        uwTick += sleepDurationMs - sleepMs;
    } else {
        uwTick += sleepMs;
    }

    rtcAlarmDisable();
    if (rtcWakeupFlag) {
        rtcWakeupFlag = false;
    }

    // Enable SysTick and update clock configuration
    extern void SystemClock_Config(void);
    SystemClock_Config();
    HAL_ResumeTick();
}

void sleepManagerProcess()
{
    // __disable_irq();
    bool isSoundPlaying = notePlayerIsPlaying();
    bool isEffectPlaying = effectManagerIsPlaying();
    bool isKeyScanRunning = keyscanIsRunning();

    if (isSoundPlaying) {
        lastPlaybackMs = HAL_GetTick();
    } else {
        if (!audioAmplifierIsMuted() && HAL_GetTick() - lastPlaybackMs >= APM_MUTE_TIMEOUT_MS) {
            audioAmplifierMute(true);
        }
    }

    if (isSoundPlaying || isKeyScanRunning || isEffectPlaying) {
        __WFI();
    } else {
        // TODO: fix state transition blocking by sleep, use state machine to handle sleep
        // enterStopMode();
    }
    // __enable_irq();
    // HAL_IWDG_Refresh(&hiwdg);
}
