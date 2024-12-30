#pragma once
#include "keys.h"

typedef enum {
    LEVEL_1,
    LEVEL_2,
    LEVEL_3,
    LEVEL_4,
    LEVEL_5,
    LEVEL_COUNT,
} LevelNumber;

typedef struct {
    const Key * const sequence;
    uint32_t keyCount;
} Level;

const Level * levelsGetStaticLevel(LevelNumber levelNum);
const Level * levelsGetRandomLevel();
void levelsGenerateRandomLevel(LevelNumber levelNum);
