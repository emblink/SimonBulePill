#include "generic.h"

bool isTimeoutHappened(uint32_t lastProcessMs, uint32_t timeoutMs)
{
    return HAL_GetTick() - lastProcessMs >= timeoutMs;
}
