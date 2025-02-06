#include "gameMenu.h"
#include "fontSize.h"
#include "oled.h"
#include "gameSettings.h"
#include "levels.h"
#include "gameStateDefines.h"
#include "gameState.h"
#include "generic.h"

#define MENU_UPDATE_INTERVAL_MS 100
#define MENU_HIGHLIGHT_INTERVAL_MS 250
typedef enum {
    MENU_ITEM_LEVEL = 0,
    MENU_ITEM_SPEED,
    MENU_ITEM_MODE,
    MENU_ITEM_SEQUENCE,
    MENU_ITEM_RESET,
    MENU_ITEM_SAVE_AND_EXIT,
    MENU_ITEM_EXIT,
    MENU_ITEM_COUNT
} MenuItem;

static const char * menuItems[] = {
    [MENU_ITEM_LEVEL] = "Level",
    [MENU_ITEM_SPEED] = "Speed",
    [MENU_ITEM_MODE] = "Mode",
    [MENU_ITEM_SEQUENCE] = "Sequence",
    [MENU_ITEM_RESET] = "Reset to defaults",
    [MENU_ITEM_SAVE_AND_EXIT] = "Save and exit",
    [MENU_ITEM_EXIT] = "Exit",
};

static const char * speedToStr[] = {
    [GAME_SPEED_SLOW] = "Slow",
    [GAME_SPEED_MEDIUM] = "Medium",
    [GAME_SPEED_HIGH] = "High",
};

static const char * modeToStr[] = {
    [GAME_MODE_SINGLE] = "Single",
    [GAME_MODE_PVP] = "1v1",
};

static const char * sequenceToStr[] = {
    [GAME_SEQUENCE_STATIC] = "Static",
    [GAME_SEQUENCE_RANDOM] = "Random",
};

static const char * levelToStr[] = {
    "1",
    "2",
    "3",
    "4",
    "5",
};

static const char ** settingValueToStr[] = {
    [MENU_ITEM_LEVEL] = levelToStr,
    [MENU_ITEM_SPEED] = speedToStr,
    [MENU_ITEM_MODE] = modeToStr,
    [MENU_ITEM_SEQUENCE] = sequenceToStr
};

static const uint8_t settingMaxValue[] = {
    [MENU_ITEM_LEVEL] = LEVEL_COUNT,
    [MENU_ITEM_SPEED] = GAME_SPEED_COUNT,
    [MENU_ITEM_MODE] = GAME_MODE_COUNT,
    [MENU_ITEM_SEQUENCE] = GAME_SEQUENCE_COUNT
};

static uint32_t lastUpdateMs = 0;
static uint32_t lastHighlightMs = 0;
static bool isHighlighted = false;
static GameSettings settings = {0};
static int currentItem = MENU_ITEM_LEVEL;
static bool isItemSelected = false;

static uint8_t * itemToSettingValue[] = {
    [MENU_ITEM_LEVEL] = &settings.level,
    [MENU_ITEM_SPEED] = &settings.speed,
    [MENU_ITEM_MODE] = &settings.mode,
    [MENU_ITEM_SEQUENCE] = &settings.sequence
};

static const char * settingsGetString(MenuItem item)
{
    if (item > MENU_ITEM_SEQUENCE) {
        return NULL;
    }

    uint8_t val = *itemToSettingValue[item];
    if (val > settingMaxValue[item]) {
        return "Invalid";
    }

    return settingValueToStr[item][val];
}

void gameMenuInit(void)
{
    gameSettingsRead(&settings);
    currentItem = MENU_ITEM_LEVEL;
    isItemSelected = false;
    isHighlighted = false;
    lastUpdateMs = HAL_GetTick();
    OLED_FillScreen(Black);
    OLED_SetCursor(0, 0);
    OLED_SetTextSize(FontSize16);
    OLED_Printf(" MENU");
    OLED_UpdateScreen();
}

static void updateMenu()
{
    OLED_FillScreen(Black);
    OLED_SetCursor(0, 0);
    OLED_SetTextSize(FontSize12);
    OLED_Printf("%s\n", menuItems[currentItem]);
    OLED_SetTextSize(FontSize12);
    const char *str = settingsGetString(currentItem);
    if (str) {
        if (isHighlighted) {
            OLED_Printf(">%s", str);
        } else {
            OLED_Printf(" %s ", str);
        }
    }
    OLED_UpdateScreen();
    lastUpdateMs = HAL_GetTick();
}

static void processSettingChange(MenuAction action) {
    if (currentItem >= MENU_ITEM_LEVEL && currentItem <= MENU_ITEM_SEQUENCE) {
        uint8_t *setting = itemToSettingValue[currentItem];
        uint8_t maxVal = settingMaxValue[currentItem];
        if (action == MENU_ACTION_UP) {
            *setting = (*setting + 1) % maxVal;
        } else if (action == MENU_ACTION_DOWN) {
            *setting = (*setting + maxVal - 1) % maxVal;
        }
    }
}

static void processSelectAction()
{
    switch (currentItem) {
        case MENU_ITEM_LEVEL:
        case MENU_ITEM_SPEED:
        case MENU_ITEM_MODE:
        case MENU_ITEM_SEQUENCE:
            isItemSelected = !isItemSelected;
            isHighlighted = isItemSelected;
            break;
        case MENU_ITEM_RESET:
            gameSettingsReset();
            gameSettingsRead(&settings);
            break;
        case MENU_ITEM_SAVE_AND_EXIT:
            gameSettingsWrite(&settings);
            gameStateProcessEvent(EVENT_MENU_EXIT);
            break;
        case MENU_ITEM_EXIT:
            gameStateProcessEvent(EVENT_MENU_EXIT);
            break;
    }
}

static void processMenuItemChange(MenuAction action)
{
    switch (action) {
    case MENU_ACTION_UP:
        currentItem = (currentItem + 1) % MENU_ITEM_COUNT;
        break;
    case MENU_ACTION_DOWN:
        currentItem = (currentItem + MENU_ITEM_COUNT - 1) % MENU_ITEM_COUNT;
        break;
    default:
        break;
    }
}

void gameMenuProcessAction(MenuAction action)
{
    if (MENU_ACTION_MENU == action) {
        gameStateProcessEvent(EVENT_MENU_EXIT);
        return;
    }

    if (MENU_ACTION_SELECT == action) {
        processSelectAction();
        return;
    }

    if (isItemSelected) {
        processSettingChange(action);
    } else {
        processMenuItemChange(action);
    }
    updateMenu();
}

void gameMenuProcess(void)
{
    bool update = false;
    if (isItemSelected && isTimeoutHappened(lastHighlightMs, MENU_HIGHLIGHT_INTERVAL_MS)) {
        isHighlighted = !isHighlighted;
        lastHighlightMs = HAL_GetTick();
        update = true;
    } else if (isTimeoutHappened(lastUpdateMs, MENU_UPDATE_INTERVAL_MS)) {
        update = true;
    }

    if (update) {
        updateMenu();
    }
}
