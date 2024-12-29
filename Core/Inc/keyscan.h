#pragma once
#include <stdbool.h>
#include "keys.h"

typedef void (* KeyCallback)(Key key, bool pressed);

void keyscanInit(KeyCallback callback);
void keyscanProcess(); // called each 1 ms from systick interrupt context
void keyscanDisableIrq(); // disables systick interrupt
void keyscanEnableIrq();
