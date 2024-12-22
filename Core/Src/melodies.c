#include "melodies.h"

#undef NOTE_DUR
#define NOTE_DUR 1000
static const Note Test[] = {
  {NOTE_A4, NOTE_DUR},
  {NOTE_B4, NOTE_DUR},
  {NOTE_C4, NOTE_DUR},
  {NOTE_D4, NOTE_DUR},
  {NOTE_G4, NOTE_DUR},
  {NOTE_F4, NOTE_DUR},
};

// https://www.tinkercad.com/things/gwf8NWvuPFV-jingle-bells-arduino
// Circuit by Soumadip Dey BWU
#undef NOTE_DUR
#define NOTE_DUR 200
static const Note JingleBells[] = {
  {NOTE_E5, NOTE_DUR}, {NOTE_E5, NOTE_DUR}, {NOTE_E5, NOTE_DUR * 2},
  {NOTE_E5, NOTE_DUR}, {NOTE_E5, NOTE_DUR}, {NOTE_E5, NOTE_DUR * 2},
  {NOTE_E5, NOTE_DUR}, {NOTE_G5, NOTE_DUR}, {NOTE_C5, NOTE_DUR}, {NOTE_D5, NOTE_DUR}, {NOTE_E5, NOTE_DUR * 2},
  {REST, NOTE_DUR * 2},
  {NOTE_F5, NOTE_DUR}, {NOTE_F5, NOTE_DUR}, {NOTE_F5, NOTE_DUR},
  {NOTE_F5, NOTE_DUR}, {NOTE_F5, NOTE_DUR}, {NOTE_E5, NOTE_DUR}, {NOTE_E5, NOTE_DUR},
  {NOTE_E5, NOTE_DUR}, {NOTE_E5, NOTE_DUR}, {NOTE_D5, NOTE_DUR}, {NOTE_D5, NOTE_DUR},
  {NOTE_E5, NOTE_DUR}, {NOTE_D5, NOTE_DUR * 2}, {NOTE_G5, NOTE_DUR * 2},
};

/*
  Hedwig's theme - Harry Potter
  More songs available at https://github.com/robsoncouto/arduino-songs
                                              Robson Couto, 2019
*/
#undef NOTE_DUR
#define NOTE_DUR 400
static const Note HarryPotter[] = {
  {REST, NOTE_DUR / 2}, {NOTE_D4, NOTE_DUR / 4},
  {NOTE_G4, NOTE_DUR / 4 + NOTE_DUR / 8}, {NOTE_AS4, NOTE_DUR / 8}, {NOTE_A4, NOTE_DUR / 4},
  {NOTE_G4, NOTE_DUR / 2}, {NOTE_D5, NOTE_DUR / 4},
  {NOTE_C5, NOTE_DUR / 2 + NOTE_DUR / 4},
  {NOTE_A4, NOTE_DUR / 2 + NOTE_DUR / 4},
  {NOTE_G4, NOTE_DUR / 4 + NOTE_DUR / 8}, {NOTE_AS4, NOTE_DUR / 8}, {NOTE_A4, NOTE_DUR / 4},
  {NOTE_F4, NOTE_DUR / 2}, {NOTE_GS4, NOTE_DUR / 4},
  {NOTE_D4, NOTE_DUR},
  {NOTE_D4, NOTE_DUR / 4},

  {NOTE_G4, NOTE_DUR / 4 + NOTE_DUR / 8}, {NOTE_AS4, NOTE_DUR / 8}, {NOTE_A4, NOTE_DUR / 4}, // 10
  {NOTE_G4, NOTE_DUR / 2}, {NOTE_D5, NOTE_DUR / 4},
  {NOTE_F5, NOTE_DUR / 2}, {NOTE_E5, NOTE_DUR / 4},
  {NOTE_DS5, NOTE_DUR / 2}, {NOTE_B4, NOTE_DUR / 4},
  {NOTE_DS5, NOTE_DUR / 4 + NOTE_DUR / 8}, {NOTE_D5, NOTE_DUR / 8}, {NOTE_CS5, NOTE_DUR / 4},
  {NOTE_CS4, NOTE_DUR / 2}, {NOTE_B4, NOTE_DUR / 4},
  {NOTE_G4, NOTE_DUR},
  {NOTE_AS4, NOTE_DUR / 4},

  {NOTE_D5, NOTE_DUR / 2}, {NOTE_AS4, NOTE_DUR / 4}, // 18
  {NOTE_D5, NOTE_DUR / 2}, {NOTE_AS4, NOTE_DUR / 4},
  {NOTE_DS5, NOTE_DUR / 2}, {NOTE_D5, NOTE_DUR / 4},
  {NOTE_CS5, NOTE_DUR / 2}, {NOTE_A4, NOTE_DUR / 4},
  {NOTE_AS4, NOTE_DUR / 4 + NOTE_DUR / 8}, {NOTE_D5, NOTE_DUR / 8}, {NOTE_CS5, NOTE_DUR / 4},
  {NOTE_CS4, NOTE_DUR / 2}, {NOTE_D4, NOTE_DUR / 4},
  {NOTE_D5, NOTE_DUR},
  {REST, NOTE_DUR / 4}, {NOTE_AS4, NOTE_DUR / 4},

  {NOTE_D5, NOTE_DUR / 2}, {NOTE_AS4, NOTE_DUR / 4}, // 26
  {NOTE_D5, NOTE_DUR / 2}, {NOTE_AS4, NOTE_DUR / 4},
  {NOTE_F5, NOTE_DUR / 2}, {NOTE_E5, NOTE_DUR / 4},
  {NOTE_DS5, NOTE_DUR / 2}, {NOTE_B4, NOTE_DUR / 4},
  {NOTE_DS5, NOTE_DUR / 4 + NOTE_DUR / 8}, {NOTE_D5, NOTE_DUR / 8}, {NOTE_CS5, NOTE_DUR / 4},
  {NOTE_CS4, NOTE_DUR / 2}, {NOTE_AS4, NOTE_DUR / 4},
  {NOTE_G4, NOTE_DUR},
};

#undef NOTE_DUR
#define NOTE_DUR 700
static const Note StarWars[] = {
    {NOTE_AS4, NOTE_DUR / 8}, {NOTE_AS4, NOTE_DUR / 8}, {NOTE_AS4, NOTE_DUR / 8}, // 1
//    {NOTE_F5, NOTE_DUR / 2}, {NOTE_C6, NOTE_DUR / 2},
//    {NOTE_AS5, NOTE_DUR / 8}, {NOTE_A5, NOTE_DUR / 8}, {NOTE_G5, NOTE_DUR / 8}, {NOTE_F6, NOTE_DUR / 2}, {NOTE_C6, NOTE_DUR / 4},
//    {NOTE_AS5, NOTE_DUR / 8}, {NOTE_A5, NOTE_DUR / 8}, {NOTE_G5, NOTE_DUR / 8}, {NOTE_F6, NOTE_DUR / 2}, {NOTE_C6, NOTE_DUR / 4},
//    {NOTE_AS5, NOTE_DUR / 8}, {NOTE_A5, NOTE_DUR / 8}, {NOTE_AS5, NOTE_DUR / 8}, {NOTE_G5, NOTE_DUR / 2},
//    {NOTE_C5, NOTE_DUR / 8}, {NOTE_C5, NOTE_DUR / 8}, {NOTE_C5, NOTE_DUR / 8},
//    {NOTE_F5, NOTE_DUR / 2}, {NOTE_C6, NOTE_DUR / 2},
//    {NOTE_AS5, NOTE_DUR / 8}, {NOTE_A5, NOTE_DUR / 8}, {NOTE_G5, NOTE_DUR / 8}, {NOTE_F6, NOTE_DUR / 2}, {NOTE_C6, NOTE_DUR / 4},
//
//    {NOTE_AS5, NOTE_DUR / 8}, {NOTE_A5, NOTE_DUR / 8}, {NOTE_G5, NOTE_DUR / 8}, {NOTE_F6, NOTE_DUR / 2}, {NOTE_C6, NOTE_DUR / 4}, // 8
//    {NOTE_AS5, NOTE_DUR / 8}, {NOTE_A5, NOTE_DUR / 8}, {NOTE_AS5, NOTE_DUR / 8}, {NOTE_G5, NOTE_DUR / 2},
//    {NOTE_C5, NOTE_DUR * 3 / 8}, {NOTE_C5, NOTE_DUR / 16},
//    {NOTE_D5, NOTE_DUR * 3 / 4}, {NOTE_D5, NOTE_DUR / 8},
//    {NOTE_AS5, NOTE_DUR / 8}, {NOTE_A5, NOTE_DUR / 8}, {NOTE_G5, NOTE_DUR / 8}, {NOTE_F5, NOTE_DUR / 8},
//    {NOTE_F5, NOTE_DUR / 8}, {NOTE_G5, NOTE_DUR / 8}, {NOTE_A5, NOTE_DUR / 8}, {NOTE_G5, NOTE_DUR / 4},
//    {NOTE_D5, NOTE_DUR / 8}, {NOTE_E5, NOTE_DUR / 4}, {NOTE_C5, NOTE_DUR * 3 / 8}, {NOTE_C5, NOTE_DUR / 16},
//    {NOTE_D5, NOTE_DUR * 3 / 4}, {NOTE_D5, NOTE_DUR / 8},
//    {NOTE_AS5, NOTE_DUR / 8}, {NOTE_A5, NOTE_DUR / 8}, {NOTE_G5, NOTE_DUR / 8}, {NOTE_F5, NOTE_DUR / 8},
//
//    {NOTE_C6, NOTE_DUR * 3 / 8}, {NOTE_G5, NOTE_DUR / 16}, {NOTE_G5, NOTE_DUR / 2}, {REST, NOTE_DUR / 8}, {NOTE_C5, NOTE_DUR / 8}, // 13
//    {NOTE_D5, NOTE_DUR * 3 / 4}, {NOTE_D5, NOTE_DUR / 8},
//    {NOTE_AS5, NOTE_DUR / 8}, {NOTE_A5, NOTE_DUR / 8}, {NOTE_G5, NOTE_DUR / 8}, {NOTE_F5, NOTE_DUR / 8},
//    {NOTE_F5, NOTE_DUR / 8}, {NOTE_G5, NOTE_DUR / 8}, {NOTE_A5, NOTE_DUR / 8}, {NOTE_G5, NOTE_DUR / 4},
//    {NOTE_D5, NOTE_DUR / 8}, {NOTE_E5, NOTE_DUR / 4}, {NOTE_C6, NOTE_DUR * 3 / 8}, {NOTE_C6, NOTE_DUR / 16},
//    {NOTE_F6, NOTE_DUR / 4}, {NOTE_DS6, NOTE_DUR / 8}, {NOTE_CS6, NOTE_DUR / 4}, {NOTE_C6, NOTE_DUR / 8},
//    {NOTE_AS5, NOTE_DUR / 4}, {NOTE_GS5, NOTE_DUR / 8}, {NOTE_G5, NOTE_DUR / 4}, {NOTE_F5, NOTE_DUR / 8},
//    {NOTE_C6, NOTE_DUR}
};

const Note * const getMelody(Melody melody)
{
	switch (melody) {
	case MelodyTest:
		return Test;

	case MelodyJingleBells:
		return JingleBells;

	case MelodyHarryPotter:
		return HarryPotter;

	case MelodyStarWars:
		return StarWars;

	default:
		return JingleBells;
	}
}

#define ELEMENTS(arr) (sizeof(arr) / sizeof((arr)[0]))

uint32_t getMelodyLength(Melody melody)
{
	switch (melody) {
	case MelodyTest:
		return ELEMENTS(Test);

	case MelodyJingleBells:
		return ELEMENTS(JingleBells);

	case MelodyHarryPotter:
		return ELEMENTS(HarryPotter);

	case MelodyStarWars:
		return ELEMENTS(StarWars);

	default:
		return 0;
	}
}
