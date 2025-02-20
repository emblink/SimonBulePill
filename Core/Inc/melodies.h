#pragma once
#include <stdint.h>
#include "notes.h"

typedef enum {
	MelodyJingleBells,
	MelodyHarryPotter,
	MelodyStarWars,
	MelodyPowerOn,
	MelodyPowerOff,
	MelodySuccess,
	MelodyFail,
	MelodyConfirm,
	MelodyCount,
	MelodyTest,
} Melody;

const Note * const getMelody(Melody melody);
uint32_t getMelodyLength(Melody melody);
