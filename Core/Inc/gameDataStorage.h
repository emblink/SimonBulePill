#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "gameSettings.h"

#define DATA_STORAGE_VERSION 1

void gameDataStorageReset();
bool gameDataStorageRead(GameSettings *settings);
bool gameDataStorageWrite(GameSettings *settings);
