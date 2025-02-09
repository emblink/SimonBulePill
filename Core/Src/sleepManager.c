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
#include "generic.h"
#include "stm32f1xx_ll_pwr.h"
#include "animationSystem.h"

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
    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
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
    audioAmplifierShutdown(true);
    rtcAlarmDisable();
    // Clear WUF bit in Power Control/Status register (PWR_CSR) upon Standby mode entry
    // This bit is set by hardware to indicate that the device received a wake-up event.
    // It is cleared by a system reset or by setting the CWUF bit in the Power control register (PWR_CR)
    LL_PWR_ClearFlag_WU();
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
    HAL_PWR_EnterSTANDBYMode();
}

void sleepManagerProcess()
{
    bool isSoundPlaying = notePlayerIsPlaying();
    bool isEffectPlaying = effectManagerIsPlaying();
    bool isKeyScanRunning = keyscanIsRunning();

    if (GAME_STATE_OFF == gameStateGetCurrentState()) {
        audioAmplifierMute(true);
        enterStandbyMode();
    }

    if (isSoundPlaying) {
        lastPlaybackMs = HAL_GetTick();
    } else {
        if (!audioAmplifierIsMuted() && isTimeoutHappened(lastPlaybackMs, APM_MUTE_TIMEOUT_MS)) {
            audioAmplifierMute(true);
        }
    }

    if (isSoundPlaying || isKeyScanRunning || isEffectPlaying) {
        __WFI();
    } else {
        uint32_t nextStateUpdate = gameStateGetNextProcessInterval();
        uint32_t nextAnimationUpdate = animationSystemGetNextUpdateInterval();
        uint32_t sleepMs = nextStateUpdate < nextAnimationUpdate ? nextStateUpdate : nextAnimationUpdate;
        if (sleepMs) {
            enterStopMode(sleepMs);
        }
    }
    // HAL_IWDG_Refresh(&hiwdg);
}
