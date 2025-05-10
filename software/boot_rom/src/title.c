#include "title.h"

bool waiting;
u8 selectedValue;
bool upPressed;
bool downPressed;

static void displayPlayerChoice();
static void handleInput();
static void joyEvent(u16 joy, u16 changed, u16 state);

u8 showTitle()
{
    waiting = TRUE;
    selectedValue = 0;
    upPressed = FALSE;
    downPressed = FALSE;

    PAL_setColors(0, (u16 *)palette_red, 16, DMA);
    PAL_setColors(16, (u16 *)palette_black, 16, DMA);
    PAL_setColors(32, (u16 *)palette_green, 16, DMA);
    PAL_setColors(48, (u16 *)palette_blue, 16, DMA);

    JOY_init();
    JOY_setEventHandler(joyEvent);

    while (waiting)
    {
        handleInput();
        displayPlayerChoice();
        SYS_doVBlankProcess();
    }

    VDP_clearText(9, 22, 8);
    VDP_clearText(9, 24, 9);

    return selectedValue + 1;
}

static void displayPlayerChoice()
{
    VDP_setTextPalette(selectedValue == 0 ? 2 : 3);
    VDP_drawText("1 player", 9, 22);
    VDP_setTextPalette(selectedValue == 1 ? 2 : 3);
    VDP_drawText("2 players", 9, 24);
}

static void handleInput()
{
    u16 value = JOY_readJoypad(JOY_1);

    if (value & BUTTON_UP)
    {
        if (!upPressed)
            selectedValue--;
        upPressed = TRUE;
        downPressed = FALSE;
    }
    else if (value & BUTTON_DOWN)
    {
        if (!downPressed)
            selectedValue++;
        upPressed = FALSE;
        downPressed = TRUE;
    }
    else
    {
        upPressed = FALSE;
        downPressed = FALSE;
    }
    selectedValue %= 2;
}

static void joyEvent(u16 joy, u16 changed, u16 state)
{
    if (changed & state & BUTTON_START)
    {
        waiting = FALSE;
    }
}
