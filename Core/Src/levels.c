#include "levels.h"
#include <stdlib.h>
#include "generic.h"

static const uint8_t level1[] = {
    KEY_GREEN, KEY_BLUE, KEY_GREEN, KEY_RED, KEY_YELLOW,
};

static const uint8_t level2[] = {
    KEY_RED, KEY_BLUE, KEY_YELLOW, KEY_GREEN, KEY_BLUE,
    KEY_GREEN, KEY_YELLOW, KEY_RED, KEY_RED, KEY_BLUE
};

static const uint8_t level3[] = {
    KEY_YELLOW, KEY_BLUE, KEY_GREEN, KEY_RED, KEY_YELLOW,
    KEY_GREEN, KEY_BLUE, KEY_RED, KEY_YELLOW, KEY_GREEN,
    KEY_BLUE, KEY_RED, KEY_YELLOW, KEY_GREEN, KEY_BLUE
};

static const uint8_t level4[] = {
    KEY_RED, KEY_BLUE, KEY_YELLOW, KEY_GREEN, KEY_BLUE, KEY_GREEN, KEY_YELLOW,
    KEY_RED, KEY_RED, KEY_BLUE, KEY_GREEN, KEY_YELLOW, KEY_YELLOW, KEY_RED,
    KEY_BLUE, KEY_GREEN, KEY_BLUE, KEY_RED, KEY_GREEN, KEY_YELLOW
};

static const uint8_t level5[] = {
    KEY_RED, KEY_BLUE, KEY_YELLOW, KEY_GREEN, KEY_BLUE, KEY_GREEN, KEY_YELLOW,
    KEY_RED, KEY_RED, KEY_BLUE, KEY_GREEN, KEY_YELLOW, KEY_YELLOW, KEY_RED,
    KEY_BLUE, KEY_GREEN, KEY_BLUE, KEY_RED, KEY_GREEN, KEY_YELLOW, KEY_RED,
    KEY_BLUE, KEY_GREEN, KEY_YELLOW, KEY_GREEN, KEY_BLUE, KEY_YELLOW, KEY_RED,
    KEY_GREEN, KEY_BLUE
};

static const Level staticLevels[LEVEL_COUNT] = {
    {level1, ELEMENTS(level1)},
    {level2, ELEMENTS(level2)},
    {level3, ELEMENTS(level3)},
    {level4, ELEMENTS(level4)},
    {level5, ELEMENTS(level5)},
};

static uint8_t levelRandomSequence[30] = {};

static Level levelRandom = {
        .sequence = levelRandomSequence,
        .keyCount = 0
};

const Level * levelsGetStaticLevel(LevelNumber levelNum)
{
    if (levelNum >= LEVEL_COUNT) {
        return NULL;
    }

    return &staticLevels[levelNum];
}

const Level * levelsGetRandomLevel()
{
    return &levelRandom;
}

void levelsGenerateRandomLevel(LevelNumber levelNum)
{
    if (levelNum >= LEVEL_COUNT) {
        return;
    }

    srand(HAL_GetTick());
    uint32_t keyCount = staticLevels[levelNum].keyCount;
    for (int i = 0; i < keyCount; i++) {
        levelRandomSequence[i] = rand() % KEY_MENU;
    }
    levelRandom.keyCount = keyCount;
}
