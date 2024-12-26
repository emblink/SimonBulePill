#include "keys.h"

const KeyPin keyTable[] = {
	[KEY_RED] = {KEY_RED_GPIO_Port, KEY_RED_Pin},
	[KEY_GREEN] = {KEY_GREEN_GPIO_Port, KEY_GREEN_Pin},
	[KEY_BLUE] = {KEY_BLUE_GPIO_Port, KEY_BLUE_Pin},
	[KEY_YELLOW] = {KEY_YELLOW_GPIO_Port, KEY_YELLOW_Pin},
};
