#pragma once
#include <stdbool.h>
#include <stdint.h>

#define ELEMENTS(arr) (sizeof(arr) / sizeof((arr)[0]))

bool isTimeoutHappened(uint32_t lastProcessMs, uint32_t timeoutMs);