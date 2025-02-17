#include <stddef.h>
#include "animationSystem.h"
#include "generic.h"
#include "oled.h"

typedef struct {
    Animation anim;
    int frameIdx;
    uint32_t lastFrameMs;
    bool repeat;
} AnimationInstance;

static AnimationInstance currentAnim = {
    .anim.frames = NULL,
};

void animationSystemPlay(AnimationName name, bool repeat)
{
    currentAnim.anim = getAnimation(name);
    currentAnim.frameIdx = 0;
    currentAnim.repeat = repeat;
    currentAnim.lastFrameMs = HAL_GetTick();
    OLED_DrawImage(currentAnim.anim.frames[currentAnim.frameIdx]);
    OLED_UpdateScreen();
}

void animationSystemStop()
{
    currentAnim.anim.frames = NULL;
}

void animationSystemProcess()
{
    if (NULL == currentAnim.anim.frames) {
        return;
    }

    if (isTimeoutHappened(currentAnim.lastFrameMs, currentAnim.anim.frameDelayMs)) {
        currentAnim.lastFrameMs = HAL_GetTick();
        OLED_DrawImage(currentAnim.anim.frames[currentAnim.frameIdx]);
        currentAnim.frameIdx = (currentAnim.frameIdx + 1) % currentAnim.anim.frameCount;
        OLED_UpdateScreen();
    }
}

bool animationSystemIsPlaying()
{
    return NULL != currentAnim.anim.frames;
}

uint32_t animationSystemGetNextUpdateInterval()
{
	if (animationSystemIsPlaying()) {
		uint32_t timeTillNextUpdate = currentAnim.lastFrameMs + currentAnim.anim.frameDelayMs - HAL_GetTick();
		return timeTillNextUpdate;
	}

	return 0;
}
