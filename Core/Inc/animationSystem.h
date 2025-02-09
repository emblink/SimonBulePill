#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "animation.h"

void animationSystemPlay(AnimationName anim, bool repeat);
void animationSystemStop();
void animationSystemProcess();
bool animationSystemIsPlaying();
uint32_t animationSystemGetNextUpdateInterval();
