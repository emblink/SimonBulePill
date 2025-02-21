#pragma once
#include <stdint.h>
#include "oled.h"

typedef enum {
    ANIM_HELLO,
    ANIM_HAPPY,
    ANIM_COOL,
    ANIM_SAD,
	ANIM_SLEEP,
    ANIM_COUNT,
} AnimationName;

typedef uint8_t AnimationFrame[(OLEDWIDTH * OLEDHEIGHT) / 8];

typedef struct {
    AnimationName name;
    const AnimationFrame *frames;
    int frameCount;
    int frameDelayMs;
} Animation;

Animation getAnimation(AnimationName anim);
