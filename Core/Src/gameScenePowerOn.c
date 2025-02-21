#include "animationSystem.h"
#include "effectManager.h"
#include "game.h"
#include "keys.h"
#include "melodies.h"
#include "notePlayer.h"

typedef enum {
    SCENE_STATE_PLAY_PWRON,
    SCENE_STATE_WAIT_INPUT,
    SCENE_STATE_PLAY_SUCCESS,
    SCENE_STATE_COUNT,
} SceneState;

static SceneState state = SCENE_STATE_PLAY_PWRON;
static bool inputDone = false;

static void onKeyPressed(Key key, bool pressed)
{
    if (!inputDone && pressed && key != KEY_MENU) {
        inputDone = true;
    }
}

void gameScenePowerOnEnter()
{
    effectManagerPlayPowerOn();
    animationSystemPlay(ANIM_HELLO, true);
    notePlayerPlayMelody(getMelody(MelodyPowerOn), getMelodyLength(MelodyPowerOn));
    inputDone = false;
    state = SCENE_STATE_PLAY_PWRON;
}

void gameScenePowerOnProcess()
{
    switch (state) {
    case SCENE_STATE_PLAY_PWRON:
        if (!effectManagerIsPlaying()) {
            state = SCENE_STATE_WAIT_INPUT;
            gameSetSceneInputCb(onKeyPressed);
        }
        break;

    case SCENE_STATE_WAIT_INPUT:
        if (inputDone) {
            state = SCENE_STATE_PLAY_SUCCESS;
            gamePlaySuccessIndicaton();
        }
        break;

    case SCENE_STATE_PLAY_SUCCESS:
        if (!effectManagerIsPlaying() && !notePlayerIsPlaying()) {
            animationSystemStop();
            gameOnSceneEnd();
        }
        break;

    default:
        break;
    }
}
