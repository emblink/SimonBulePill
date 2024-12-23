#include "keyscan.h"
#include <stddef.h>

#define US_TIMER 0
#define PROCESS_COUNT (20)
#define PRESSED_THRESHOLD (5)
#define RELEASED_THRESHOLD (-5)

static volatile int processCounter = 0;
static KeyCallback keyCb = NULL;

static int keyThreshold[KEY_COUNT] = {0};
static bool keyState[KEY_COUNT] = {false};

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
				if (true == keyState[key]) {
					keyState[key] = false;
					keyCb(key, keyState[key]);
				}
			}
		} else {
			keyThreshold[key]--;
			if (keyThreshold[key] <= RELEASED_THRESHOLD) {
				if (false == keyState[key]) {
					keyState[key] = true;
					keyCb(key, keyState[key]);
				}
			}
		}
	}
	processCounter--;
}



