#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "notes.h"

typedef void (* PlaybackCb)(void);
typedef void (* NoteStartCb)(uint32_t noteHz, uint32_t durationMs);

void notePlayerInit(NoteStartCb noteStart, PlaybackCb finish);
void notePlayerPlayNote(uint32_t note, uint32_t durationMs);
void notePlayerPlayMelody(const Note melody[], uint32_t length);
bool notePlayerIsPlaying();
void notePlayerStop();
