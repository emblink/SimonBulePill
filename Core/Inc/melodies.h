#pragma once
#include <stdint.h>
#include "notes.h"

typedef enum {
	MelodyJingleBells,
	MelodyHarryPotter,
	MelodyStarWars,
	MelodyCount,
	MelodyTest,
} Melody;

const Note * const getMelody(Melody melody);
uint32_t getMelodyLength(Melody melody);
