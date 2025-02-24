#pragma once
#include <stdint.h>
#include "notes.h"

typedef enum {
	MelodyJingleBells,
	MelodyHarryPotter,
	MelodyStarWars,
	MelodyDvaVeselihGusya,
	MelodyPowerOn,
	MelodyPowerOff,
	MelodySuccess,
	MelodyFail,
	MelodyConfirm,
	MelodyIdle,
	MelodyCount,
	MelodyTest,
} Melody;

const Note * const getMelody(Melody melody);
uint32_t getMelodyLength(Melody melody);
