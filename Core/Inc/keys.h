#pragma once
#include "main.h"
#include "stm32f103xb.h"

typedef struct {
	GPIO_TypeDef * port;
	uint16_t pin;
} KeyPin;

typedef enum {
	KEY_RED,
	KEY_GREEN,
	KEY_BLUE,
	KEY_YELLOW,
    KEY_MENU,
	KEY_COUNT,
} Key;

extern const KeyPin keyTable[KEY_COUNT];
