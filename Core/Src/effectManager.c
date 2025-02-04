#include "effectManager.h"
#include "notePlayer.h"
#include "tim.h"
#include "generic.h"

#define EFFECT_MAXIMUM_FRAMES 5

typedef struct {
    uint32_t startTime;  // Start time in milliseconds (relative)
    uint32_t duration;   // Duration of the transition
    uint8_t startValue;  // Starting LED brightness (0–255)
    uint8_t endValue;    // Ending LED brightness (0–255)
} EffectTransition;

typedef struct {
    Effect effect;
    uint32_t startTime;
    uint32_t totalDuration;
    uint32_t lastUpdateMs;    // Last update timestamp
    uint8_t currentTrans;  // Index of the current transition
    uint8_t totalTransNum;    // Total number of transitions
    EffectTransition trans[EFFECT_MAXIMUM_FRAMES];
} EffectSettings;

static EffectFinishedCallback finishedCb = NULL;
static EffectSettings effects[LED_COUNT] = {0};

uint8_t interpolate(int32_t start, int32_t end, uint32_t elapsed, uint32_t duration) {
    if (elapsed >= duration) {
        return end; // Return end value if elapsed time exceeds duration
    }

    // Cast to signed type for proper arithmetic handling
    int32_t delta = end - start;

    // Compute the interpolated value
    return start + (delta * (int32_t)elapsed) / (int32_t)duration;
}

static void setLedPwm(Led led, int pwm)
{
	pwm = 255 - pwm; // Invert PWM value (0 = full brightness)
	switch (led) {
	case LED_RED: TIM3->CCR1 = pwm; break;
	case LED_GREEN: TIM3->CCR2 = pwm; break;
	case LED_BLUE: TIM3->CCR3 = pwm; break;
	case LED_YELLOW: TIM3->CCR4 = pwm; break;
	default: break;
	}
}

void effectManagerInit(EffectFinishedCallback callback)
{
	finishedCb = callback;
    for (Led led = LED_RED; led < LED_COUNT; led++) {
        setLedPwm(led, 0); // Initialize all LEDs to off
    }
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
}

void effectManagerProcess(void)
{
    for (int i = 0; i < ELEMENTS(effects); i++) {
        if (EFFECT_NONE == effects[i].effect) {
            continue; // Skip inactive effects
        }

        EffectSettings *effect = &effects[i];
        uint32_t tick = HAL_GetTick();

        if (0 == effect->startTime) {
        	effect->startTime = tick;
        	uint32_t transTime = 0;
        	for (int i = 0; i < effect->totalTransNum; i++) {
        		effect->trans[i].startTime = effect->startTime + transTime;
        		transTime += effect->trans[i].duration;
        	}
        }

        // Check if the total effect duration has elapsed
        if (0 != effect->totalDuration) {
        	uint32_t effectElapsed = tick - effect->startTime;
        	if (effectElapsed > effect->totalDuration) {
                effect->effect = EFFECT_NONE; // End effect
                finishedCb(i);
                effectManagerStopEffect(i);
                continue;
        	}
        }

        EffectTransition *trans = &effect->trans[effect->currentTrans];
        // transition starting now
        uint32_t transElapsed = tick - trans->startTime;

        if (transElapsed >= trans->duration) {
            // Move to the next transition
            effect->currentTrans++;
            if (effect->currentTrans >= effect->totalTransNum) {
            	if (0 == effect->totalDuration) {
            		// restart effect
            		effect->startTime = 0;
            		effect->currentTrans = 0;
            		continue;
            	} else if (tick - effect->startTime < effect->totalDuration) {
            		// restart transitions
                	uint32_t transTime = 0;
                	for (int i = 0; i < effect->totalTransNum; i++) {
                		effect->trans[i].startTime = tick + transTime;
                		transTime += effect->trans[i].duration;
                	}
            		effect->currentTrans = 0;
            		continue;
            	} else {
            		effectManagerStopEffect(i);
                    finishedCb(i);
                    continue;
            	}
            }

            effect->lastUpdateMs = tick; // Update timestamp for the new transition
            trans = &effect->trans[effect->currentTrans]; // Load the next transition
            transElapsed = 0; // Reset elapsed time for the new transition
        }

        // Interpolate LED brightness within the current transition
        uint8_t brightness = interpolate(
            trans->startValue,
            trans->endValue,
            transElapsed,
            trans->duration
        );

        // Update the LED brightness (custom function, assuming `i` maps to LED)
        setLedPwm(i, brightness);
    }
}

static void setEffect(Effect effect, Led led, uint32_t duration, uint32_t period)
{
	if (EFFECT_BREATHE == effect) {
		effects[led] = (EffectSettings) {
			.effect = EFFECT_BREATHE,
			.startTime = 0,
			.totalDuration = duration,
			.lastUpdateMs = 0,
			.currentTrans = 0,
			.totalTransNum = 2,
			.trans = {
				{ .startTime = 0, .duration = period / 2, .startValue = 0, .endValue = 255 },  // Ramp up
				{ .startTime = 0, .duration = period / 2, .startValue = 255, .endValue = 0 } // Ramp down
			}
		};
	} else if (EFFECT_FAST_RUMP == effect) {
		#define RUMP_UP 25
		#define STATIC 50
		#define RUMP_DOWN 25
		const uint32_t rumpUpTime = period * RUMP_UP / 100;
		const uint32_t staticTime = period * STATIC / 100;
		const uint32_t rumpDownTime = period * RUMP_DOWN / 100;
		effects[led] = (EffectSettings) {
			.effect = EFFECT_BREATHE,
			.startTime = 0,
			.totalDuration = duration,
			.lastUpdateMs = 0,
			.currentTrans = 0,
			.totalTransNum = 3,
			.trans = {
				{ .startTime = 0, .duration = rumpUpTime, .startValue = 0, .endValue = 255 },  // Ramp up
				{ .startTime = 0, .duration = staticTime, .startValue = 255, .endValue = 255 },  // Static
				{ .startTime = 0, .duration = rumpDownTime, .startValue = 255, .endValue = 0 } // Ramp down
			}
		};
	} else if (EFFECT_BLINK == effect) {
		effects[led] = (EffectSettings) {
			.effect = EFFECT_BLINK,
			.startTime = 0,
			.totalDuration = duration,
			.lastUpdateMs = 0,
			.currentTrans = 0,
			.totalTransNum = 2,
			.trans = {
				{ .startTime = 0, .duration = period / 2, .startValue = 255, .endValue = 255 }, // ON
				{ .startTime = 0, .duration = period / 2, .startValue = 0, .endValue = 0 } // OFF
			}
		};
	} else if (EFFECT_STATIC == effect) {
		effects[led] = (EffectSettings) {
			.effect = EFFECT_STATIC,
			.startTime = 0,
			.totalDuration = duration,
			.lastUpdateMs = 0,
			.currentTrans = 0,
			.totalTransNum = 1,
			.trans = {
				{ .startTime = 0, .duration = duration, .startValue = 255, .endValue = 255 }, // ON
			}
		};
	}
}
void effectManagerPlayEffect(Effect effect, Led led, uint32_t duration, uint32_t period)
{
	if (LED_ALL == led) {
		for (int i = 0; i < LED_COUNT; i++) {
			setEffect(effect, i, duration, period);
		}
	} else {
		setEffect(effect, led, duration, period);
	}
}

void effectManagerPlayPowerOn()
{
	int delay = 1;
    for (int i = 0; i <= 250; i++) { // Ramp up (0-255 PWM value)
    	setLedPwm(LED_RED, i);
        HAL_Delay(delay); // Adjust for smoother ramp
    }
    for (int i = 0; i <= 250; i++) {
    	setLedPwm(LED_BLUE, i);
        HAL_Delay(delay);
    }
    for (int i = 0; i <= 250; i++) {
    	setLedPwm(LED_YELLOW, i);
        HAL_Delay(delay);
    }
    for (int i = 0; i <= 250; i++) {
    	setLedPwm(LED_GREEN, i);
        HAL_Delay(delay);
    }
    HAL_Delay(250); // Hold full brightness
    for (int i = 250; i > 0; i--) { // Ramp down
    	setLedPwm(LED_RED, i);
    	setLedPwm(LED_BLUE, i);
    	setLedPwm(LED_YELLOW, i);
    	setLedPwm(LED_GREEN, i);
        HAL_Delay(delay); // Adjust for smoother ramp
    }
}

void effectManagerPlayPowerOff()
{

}

void effectManagerPlaySuccess(uint32_t duration)
{

}

void effectManagerPlayFail(uint32_t duration)
{

}

void effectManagerStopEffect(Led led)
{
	effects[led].effect = EFFECT_NONE;
	setLedPwm(led, 0);
}

void effectManagerStopAllEffects()
{
    for (int i = 0; i < ELEMENTS(effects); i++) {
        effects[i].effect = EFFECT_NONE;
    }
	setLedPwm(LED_RED, 0);
	setLedPwm(LED_BLUE, 0);
	setLedPwm(LED_YELLOW, 0);
	setLedPwm(LED_GREEN, 0);
}

bool effectManagerIsPlaying(void)
{
	for (int i = 0; i < LED_COUNT; i++) {
		if (EFFECT_NONE != effects[i].effect) {
			return true;
		}
	}
	return false;
}
