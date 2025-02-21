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

typedef union {
    struct {
        uint8_t red : 1;
        uint8_t green : 1;
        uint8_t blue : 1;
        uint8_t yellow : 1;
        uint8_t menu : 1;
    };
    uint32_t state;
} Keys;

extern const KeyPin keyTable[KEY_COUNT];
