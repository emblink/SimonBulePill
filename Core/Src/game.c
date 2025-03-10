#include "gameIncludes.h"

#define BATTERY_CHECK_PERIOD (MINUTE * 5)
#define IDLE_STATE_TIMEOUT   (MINUTE)

typedef struct {
    bool isRunning;
    GameScene currentScene;
    GameSettings settings;
    int32_t batteryCharge;
    uint32_t lastBatteryCheckMs;
    volatile Keys input;
    SceneInputCb sceneInputCb;
    uint32_t lastInputMs;
} GameHandle;

static GameHandle game = {0};
// Nessesary

// Optional
// TODO: Add cat cool animation on level completed
// TODO: Add a game mode single player, player vs player
// TODO: Add power off animation

static void showBatteryCharge()
{
    OLED_SetCursor(105, 0);
    OLED_SetTextSize(FontSize12);
    OLED_Printf("%i", game.batteryCharge);
}

static void onPlaybackStarted(uint32_t noteHz, uint32_t duration)
{
    (void) noteHz;
    (void) duration;
    audioAmplifierMute(false);
}

static void onKeyPressCallback(Key key, bool isPressed)
{
    switch (key) {
    case KEY_RED: game.input.red = isPressed; break;
    case KEY_GREEN: game.input.green = isPressed; break;
    case KEY_BLUE: game.input.blue = isPressed; break;
    case KEY_YELLOW: game.input.yellow = isPressed; break;
    case KEY_MENU: game.input.menu = isPressed; break;
    default: break;
    }
}

static void changeScene(GameScene newScene)
{
    effectManagerStopAllEffects();
    animationSystemStop();
    notePlayerStop();
    notePlayerInit(onPlaybackStarted, NULL);

    switch (newScene) {
    case GAME_SCENE_POWER_ON: gameScenePowerOnEnter(); break;
    case GAME_SCENE_SIMON: gameSceneSimonEnter(); break;
    case GAME_SCENE_MUSIC: gameSceneMusicEnter(); break;
    case GAME_SCENE_MENU: gameSceneMenuEnter(); break;
    case GAME_SCENE_IDLE: gameSceneIdleEnter(); break;
    case GAME_SCENE_POWER_OFF: gameScenePowerOffEnter(); break;
    default: break;
    }
    game.currentScene = newScene;
}

static void processIdleTransition()
{
    if (game.currentScene == GAME_SCENE_SIMON) {
        return;
    }

    if (isTimeoutHappened(game.lastInputMs, IDLE_STATE_TIMEOUT)) {
        game.lastInputMs = HAL_GetTick(); // reset idle timer to avoid immediate power off transition
        if (GAME_SCENE_IDLE == game.currentScene) {
            changeScene(GAME_SCENE_POWER_OFF);
        } else {
            changeScene(GAME_SCENE_IDLE);
        }
    }
}

static void sceneProcess()
{
    processIdleTransition();

    switch (game.currentScene) {
    case GAME_SCENE_POWER_ON: gameScenePowerOnProcess(); break;
    case GAME_SCENE_SIMON: gameSceneSimonProcess(); break;
    case GAME_SCENE_MUSIC: gameSceneMusicProcess(); break;
    case GAME_SCENE_MENU: gameSceneMenuProcess(); break;
    case GAME_SCENE_IDLE: gameSceneIdleProcess(); break;
    case GAME_SCENE_POWER_OFF: gameScenePowerOffProcess(); break;
    default: break;
    }
}

static void onMenuKeyPress()
{
    switch (game.currentScene) {
    case GAME_SCENE_SIMON:
    case GAME_SCENE_MUSIC:
        changeScene(GAME_SCENE_MENU);
        break;

    case GAME_SCENE_MENU:
        gameSettingsRead(&game.settings);
        if (GAME_MODE_MUSIC == game.settings.mode) {
            changeScene(GAME_SCENE_MUSIC);
        } else {
            changeScene(GAME_SCENE_SIMON);
        }
        break;

    default:
        break;
    }
}

static void userInputProcess()
{
    // Critical section to avoid concurrent access to the input variable
    keyscanDisableIrq();
    Keys userInput = game.input; // copy state, should be atomic operation
    game.input.state &= ~userInput.state; // reset input state bits
    keyscanEnableIrq();

    if (userInput.state) {
        game.lastInputMs = HAL_GetTick();
    }
    
    if (userInput.menu) {
        switch (game.currentScene) {
        case GAME_SCENE_SIMON:
        case GAME_SCENE_MUSIC:
        case GAME_SCENE_MENU:
            onMenuKeyPress(); break;
        default: break;
        }
    } else if (game.sceneInputCb) {
        // report only key press events for now
        for (int i = 0; i < KEY_COUNT; i++) {
            if (userInput.state & (1 << i)) {
                if (game.sceneInputCb) {
                    game.sceneInputCb(i, true);
                }
            }
        }
    }
}

static void batteryProcess(void)
{
    if (isTimeoutHappened(game.lastBatteryCheckMs, BATTERY_CHECK_PERIOD)) {
        game.lastBatteryCheckMs = HAL_GetTick();
        game.batteryCharge = batteryManagerGetPercent();
        if (game.batteryCharge <= 0) {
            changeScene(GAME_SCENE_POWER_OFF);
        }
    }
}

void gameInit()
{
    game.isRunning = true;
    audioAmplifierInit();
    effectManagerInit(NULL);
    OLED_Init(&hi2c1);
    OLED_FillScreen(Black);
    OLED_UpdateScreen();
    keyscanInit(onKeyPressCallback);
    game.batteryCharge = batteryManagerGetPercent();
    if (!gameSettingsRead(&game.settings)) {
        gameSettingsReset();
        gameSettingsRead(&game.settings);
    }
    showBatteryCharge();
    if (game.batteryCharge <= 0) {
        changeScene(GAME_SCENE_POWER_OFF);
    } else {
        changeScene(GAME_SCENE_POWER_ON);
    }
}

void gameProcess()
{
    userInputProcess();
    sceneProcess();
    effectManagerProcess();
    animationSystemProcess();
    batteryProcess();
}

void gamePlaySuccessIndicaton()
{
    effectManagerPlaySuccess();
    notePlayerPlayMelody(getMelody(MelodySuccess), getMelodyLength(MelodySuccess));
}

void gamePlayFailedIndication()
{
    effectManagerPlayFail();
    notePlayerPlayMelody(getMelody(MelodyFail), getMelodyLength(MelodyFail));
}

void gameSetSceneInputCb(SceneInputCb cb)
{
    game.sceneInputCb = cb;
}

void gameOnSceneEnd()
{
    switch (game.currentScene) {
    case GAME_SCENE_POWER_ON:
    case GAME_SCENE_MENU:
    case GAME_SCENE_IDLE:
        gameSettingsRead(&game.settings);
        if (GAME_MODE_MUSIC == game.settings.mode) {
            changeScene(GAME_SCENE_MUSIC);
        } else {
            changeScene(GAME_SCENE_SIMON);
        }
        break;

    case GAME_SCENE_SIMON:
        changeScene(GAME_SCENE_IDLE);
        break;

    case GAME_SCENE_POWER_OFF:
        game.isRunning = false;
        break;
    
    default: assert(0);
        break;
    }
}

bool gameIsRunning()
{
    return game.isRunning;
}

uint32_t gameGetSleepDuration()
{
    uint32_t sleepMs = 0;
    uint32_t nextAnimationUpdate = animationSystemGetNextUpdateInterval();
    uint32_t idleTransitionMs = game.lastInputMs + IDLE_STATE_TIMEOUT - HAL_GetTick();

    switch (game.currentScene) {
    case GAME_SCENE_MUSIC:
    case GAME_SCENE_IDLE:
    case GAME_SCENE_MENU: {
        sleepMs = nextAnimationUpdate < idleTransitionMs ? nextAnimationUpdate : idleTransitionMs;
    } break;

    case GAME_SCENE_SIMON: {
        uint32_t nextStateUpdate = gameStateGetNextProcessInterval();
        if (nextAnimationUpdate) {
            sleepMs = nextStateUpdate < nextAnimationUpdate ? nextStateUpdate : nextAnimationUpdate;
        } else {
            sleepMs = nextStateUpdate;
        }
    } break;

    default: break;
    }

    return sleepMs;
}
