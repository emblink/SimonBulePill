#include "gameMenu.h"
#include "fontSize.h"
#include "oled.h"

static const char * menuItems[] = {
    "Level",
    "Speed",
    "Mode",
    "Sequence",
    "Reset",
    "Apply/Exit",
};

void gameMenuInit(void)
{
    OLED_FillScreen(Black);
    OLED_SetCursor(0, 0);
    OLED_SetTextSize(FontSize24);
    OLED_Printf(" MENU");
    OLED_UpdateScreen();
}

void gameMenuProcessAction(MenuAction action)
{
    OLED_FillScreen(Black);
    OLED_SetCursor(0, 0);
    OLED_SetTextSize(FontSize24);
    OLED_Printf(" MENU");
    OLED_UpdateScreen();
}

void gameMenuProcess(void)
{

}

void gameMenuExit(void)
{

}