#include "generic.h"

bool isTimeoutHappened(uint32_t lastProcessMs, uint32_t timeoutMs)
{
    uint32_t currentTick = HAL_GetTick();
    return currentTick - lastProcessMs >= timeoutMs;
}
