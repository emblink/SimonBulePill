#pragma once
#include <stdint.h>
#include "leds.h"

typedef void (* EffectFinishedCallback)(Led led);

typedef enum {
	EFFECT_NONE,
	EFFECT_STATIC,
	EFFECT_BLINK,
	EFFECT_FAST_BLINK,
	EFFECT_BREATHE,
	EFFECT_FAST_BREATHE,
	EFFECT_COUNT
} Effect;

void effectManagerInit(EffectFinishedCallback callback);
void effectManagerProcess();
void effectManagerPlayEffect(Effect effect, Led led, uint32_t duration);
void effectManagerPlayPowerOn();
void effectManagerPlayPowerOff();
void effectManagerPlaySuccess();
void effectManagerPlayFail();
void effectManagerStopEffect(Led led);
void effectManagerStopAllEffects();
