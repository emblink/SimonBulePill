#pragma once
#include <stdbool.h>
#include "keys.h"

typedef void (* SceneInputCb)(Key key, bool isPressed);

void gameInit();
bool gameIsRunning();
void gameProcess();
void gameOnSceneEnd();
void gamePlaySuccessIndicaton();
void gamePlayFailedIndication();
void gameSetSceneInputCb(SceneInputCb cb);
uint32_t gameGetSleepDuration();