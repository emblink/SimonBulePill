#pragma once
#include <stdbool.h>
#include <stdint.h>

#define SECOND 1000
#define MINUTE (60 * SECOND)

#define ELEMENTS(arr) (sizeof(arr) / sizeof((arr)[0]))

bool isTimeoutHappened(uint32_t lastProcessMs, uint32_t timeoutMs);