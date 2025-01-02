#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "notes.h"

typedef void (* PlaybackCb)(void);

void notePlayerInit(PlaybackCb start, PlaybackCb finish);
void notePlayerPlayNote(uint32_t note, uint32_t durationMs);
void notePlayerPlayMelody(const Note melody[], uint32_t length);
bool notePlayerIsPlaying();
void notePlayerStop();
