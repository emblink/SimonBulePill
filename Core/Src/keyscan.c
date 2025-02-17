#include "keyscan.h"
#include <stddef.h>
#include "generic.h"

#define US_TIMER 0
#define PROCESS_COUNT (20)
#define PRESSED_THRESHOLD (5)
#define RELEASED_THRESHOLD (-5)
#define DEBOUNCE_MS (20)

static volatile int processCounter = 0;
static KeyCallback keyCb = NULL;

static int keyThreshold[KEY_COUNT] = {0};
static bool keyIsPressed[KEY_COUNT] = {false};
static uint32_t pressDebounce[KEY_COUNT] = {0};
static uint32_t releaseDebounce[KEY_COUNT] = {0};

//  VCC __/ _____ INPUT_PIN
//            |
//            |
//            -
//           | | Pull Down Resistor
//            -
//            |
//           GND
// Pressed: 1, Released: 0

// EXTI Line9 External Interrupt ISR Handler CallBackFun
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	processCounter = PROCESS_COUNT;
}

void keyscanInit(KeyCallback callback)
{
	keyCb = callback;
}

void keyscanProcess()
{
	if (processCounter <= 0) {
		return;
	}

	if (PROCESS_COUNT == processCounter) {
		for (int key = KEY_RED; key < KEY_COUNT; key++) {
			keyThreshold[key] = 0;
		}
	}

	for (int key = KEY_RED; key < KEY_COUNT; key++) {
		GPIO_PinState state = HAL_GPIO_ReadPin(keyTable[key].port, keyTable[key].pin);
		if (GPIO_PIN_SET == state) {
			keyThreshold[key]++;
			if (keyThreshold[key] >= PRESSED_THRESHOLD) {
                if (!keyIsPressed[key]) {
                    bool debounced = isTimeoutHappened(pressDebounce[key], DEBOUNCE_MS);
                    if (keyCb && debounced) {
                        pressDebounce[key] = HAL_GetTick();
                        keyIsPressed[key] = true;
                        keyCb(key, keyIsPressed[key]);   
                    }
                }
			}
		} else {
			keyThreshold[key]--;
			if (keyThreshold[key] <= RELEASED_THRESHOLD) {
                if (keyIsPressed[key]) {
                    bool debounced = isTimeoutHappened(releaseDebounce[key], DEBOUNCE_MS);
                    if (keyCb && debounced) {
                        releaseDebounce[key] = HAL_GetTick();
                        keyIsPressed[key] = false;
                        keyCb(key, keyIsPressed[key]);
                    }
                }
			}
		}
	}
	processCounter--;
}

void keyscanDisableIrq()
{
    // Disable SysTick interrupt (the keyscan happens there)
    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
}

void keyscanEnableIrq()
{
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
}

bool keyscanIsRunning()
{
	return 0 != processCounter;
}
