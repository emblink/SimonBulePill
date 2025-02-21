#pragma once
#include "gameScenePowerOn.h"
#include "gameSceneSimon.h"
#include "gameSceneMusic.h"
#include "gameSceneMenu.h"
#include "gameSceneIdle.h"
#include "gameScenePowerOff.h"

typedef enum {
    GAME_SCENE_POWER_ON,
    GAME_SCENE_SIMON,
    GAME_SCENE_MUSIC,
    GAME_SCENE_MENU,
    GAME_SCENE_IDLE,
    GAME_SCENE_POWER_OFF,
    GAME_SCENE_COUNT,
} GameScene;