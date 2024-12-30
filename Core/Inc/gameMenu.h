#pragma once

typedef enum {
    MENU_ACTION_UP,
    MENU_ACTION_DOWN,
    MENU_ACTION_SELECT,
    MENU_ACTION_BACK,
    MENU_ACTION_MENU,
    MENU_ACTION_COUNT
} MenuAction;

void gameMenuInit(void);
void gameMenuProcessAction(MenuAction action);
void gameMenuProcess(void);
void gameMenuExit(void);