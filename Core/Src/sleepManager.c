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
#include "gameState.h"

#define APM_MUTE_TIMEOUT_MS 250

static uint32_t lastPlaybackMs = 0;
static volatile bool rtcWakeupFlag = false;

static inline void sleepPinOn()
{
	HAL_GPIO_WritePin(SLEEP_TRACK_PIN_GPIO_Port, SLEEP_TRACK_PIN_Pin, 1);
}

static inline void sleepPinOff()
{
	HAL_GPIO_WritePin(SLEEP_TRACK_PIN_GPIO_Port, SLEEP_TRACK_PIN_Pin, 0);
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

static void enterStopMode(uint32_t sleepDurationMs)
{
    HAL_SuspendTick();
    // Reset RTC time to zero
    RTC_TimeTypeDef sTime = {0};
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    rtcAlarmEnable(sleepDurationMs);
    sleepPinOff();
    // Enter Stop Mode
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
    // Exit Stop Mode
    sleepPinOn();

    // Update SysTick counter after sleep wake up
    RTC_TimeTypeDef wakeUpTime = {0};
    HAL_RTC_GetTime(&hrtc, &wakeUpTime, RTC_FORMAT_BIN);
    uint32_t sleepMs = wakeUpTime.Hours * 3600 + wakeUpTime.Minutes * 60 + wakeUpTime.Seconds;
    extern __IO uint32_t uwTick;
    uwTick += sleepMs; // add sleep time to current time

    rtcAlarmDisable();
    if (rtcWakeupFlag) {
        rtcWakeupFlag = false;
    }

    // Enable SysTick and update clock configuration
    extern void SystemClock_Config(void);
    SystemClock_Config();
    HAL_ResumeTick();
}

static void enterStandbyMode()
{

}

void sleepManagerProcess()
{
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

    uint32_t sleepMs = gameStateGetNextProcessInterval();
    if (isSoundPlaying || isKeyScanRunning || isEffectPlaying) {
        __WFI();
    } else {
        if (sleepMs) {
            enterStopMode(sleepMs);
        }
    }
    // HAL_IWDG_Refresh(&hiwdg);
}
