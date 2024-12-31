#include "gameMenu.h"
#include "fontSize.h"
#include "oled.h"
#include "gameSettings.h"

#define MENU_UPDATE_INTERVAL_MS 100
typedef enum {
    MENU_ITEM_LEVEL = 0,
    MENU_ITEM_SPEED,
    MENU_ITEM_MODE,
    MENU_ITEM_SEQUENCE,
    MENU_ITEM_RESET,
    MENU_ITEM_APPLY,
    MENU_ITEM_EXIT,
    MENU_ITEM_COUNT
} MenuIntem;

static uint32_t lastUpdateMs = 0;
static GameSettings settings = {0};
static int currentItem = MENU_ITEM_LEVEL;
static const char * menuItems[] = {
    [MENU_ITEM_LEVEL] = "Level",
    [MENU_ITEM_SPEED] = "Speed",
    [MENU_ITEM_MODE] = "Mode",
    [MENU_ITEM_SEQUENCE] = "Sequence",
    [MENU_ITEM_RESET] = "Reset",
    [MENU_ITEM_APPLY] = "Apply",
    [MENU_ITEM_EXIT] = "Exit",
};

static uint8_t menuItemGetValue(MenuIntem item)
{
    switch (item) {
        case MENU_ITEM_LEVEL:
            return settings.level + 1;
        case MENU_ITEM_SPEED:
            return settings.speed;
        case MENU_ITEM_MODE:
            return settings.mode;
        case MENU_ITEM_SEQUENCE:
            return settings.sequence;
        default:
            return 0;
    }
}

void gameMenuInit(void)
{
    gameSettingsRead(&settings);
    currentItem = MENU_ITEM_LEVEL;
    lastUpdateMs = HAL_GetTick();
    OLED_FillScreen(Black);
    OLED_SetCursor(0, 0);
    OLED_SetTextSize(FontSize24);
    OLED_Printf(" MENU");
    OLED_UpdateScreen();
}

static void updateMenu()
{
    OLED_FillScreen(Black);
    OLED_SetCursor(0, 0);
    OLED_SetTextSize(FontSize16);
    OLED_Printf("%s\n", menuItems[currentItem]);
    OLED_SetTextSize(FontSize12);
    int32_t val = menuItemGetValue(currentItem);
    OLED_Printf("%i", val);
    OLED_UpdateScreen();
    lastUpdateMs = HAL_GetTick();
}

void gameMenuProcessAction(MenuAction action)
{
    switch (action) {
        case MENU_ACTION_UP:
            currentItem++;
            if (currentItem >= MENU_ITEM_COUNT) {
                currentItem = MENU_ITEM_LEVEL;
            }
            break;
        case MENU_ACTION_DOWN:
            currentItem--;
            if (currentItem < MENU_ITEM_LEVEL) {
                currentItem = MENU_ITEM_COUNT - 1;
            }
            break;
        case MENU_ACTION_SELECT:
            switch (currentItem) {
                case MENU_ITEM_LEVEL:
                    settings.level++;
                    break;
                case MENU_ITEM_SPEED:
                    settings.speed++;
                    break;
                case MENU_ITEM_MODE:
                    settings.mode++;
                    break;
                case MENU_ITEM_SEQUENCE:
                    settings.sequence++;
                    break;
                case MENU_ITEM_RESET:
                    gameSettingsReset();
                    gameSettingsRead(&settings);
                    break;
                case MENU_ITEM_APPLY:
                    gameSettingsWrite(&settings);
                    // gameStateTransition(EVENT_INPUT_MENU);
                    break;
                case MENU_ITEM_EXIT:

                    break;
            }
            break;
        case MENU_ACTION_BACK:
            // gameStateTransition(EVENT_INPUT_MENU);
            break;
        case MENU_ACTION_MENU:
            // gameStateTransition(EVENT_INPUT_MENU);
            break;
        default:
            break;
    }

    updateMenu();
}

void gameMenuProcess(void)
{
    uint32_t tick = HAL_GetTick();
    if (tick - lastUpdateMs >= MENU_UPDATE_INTERVAL_MS) {
        updateMenu();
    }
}
