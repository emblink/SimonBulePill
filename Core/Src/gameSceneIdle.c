#include "effectManager.h"
#include "game.h"
#include "keys.h"
#include "notePlayer.h"
#include "melodies.h"
#include "animationSystem.h"

typedef enum {
    SCENE_STATE_WAIT_INPUT,
    SCENE_STATE_PLAY_SUCCESS,
    SCENE_STATE_COUNT,
} SceneState;

static SceneState state = SCENE_STATE_WAIT_INPUT;
static bool inputDone = false;

static void onKeyPressed(Key key, bool pressed)
{
    if (!inputDone && pressed && key != KEY_MENU) {
        inputDone = true;
    }
}

void gameSceneIdleEnter()
{
    effectManagerStopAllEffects();
    notePlayerPlayMelody(getMelody(MelodyIdle), getMelodyLength(MelodyIdle));
    animationSystemPlay(ANIM_SLEEP, true);
    state = SCENE_STATE_WAIT_INPUT;
    inputDone = false; // reset input state
    gameSetSceneInputCb(onKeyPressed);
}

void gameSceneIdleProcess()
{
    switch (state) {
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
