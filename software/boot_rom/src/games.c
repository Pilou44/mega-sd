#include "games.h"
#include "menu_memory.h"

#define GAMES_PER_PAGE 10

u16 selectedGame;
u8 page;
//bool upPressed;
//bool downPressed;

static void displayGamePage();
static void handleInput();
static void printGameName(u16 position);

void showGames() {
    page = 0;
    selectedGame = 0;
    //upPressed = FALSE;
    //downPressed = FALSE;

    
    PAL_setColors(0, (u16 *)palette_red, 16, DMA);
    PAL_setColors(16, (u16 *)palette_black, 16, DMA);
    PAL_setColors(32, (u16 *)palette_green, 16, DMA);
    PAL_setColors(48, (u16 *)palette_blue, 16, DMA);

    //JOY_init();
    //JOY_setEventHandler(joyEvent);

    
    while (TRUE) {
        //handleInput();
        displayGamePage();
        SYS_doVBlankProcess();
    }
}

static void displayGamePage() {
    u16 startIndex = page * GAMES_PER_PAGE;
    u16 lastIndex = menu_gamecount - 1;
    u16 pageLastIndex = (page +1) * GAMES_PER_PAGE - 1;
    u16 endIndex = (pageLastIndex > lastIndex) ? lastIndex : pageLastIndex;
    u16 pageCount = lastIndex / GAMES_PER_PAGE + 1;

    VDP_setTextPalette(2);

    char buffer[32];
    sprintf(buffer, "%d games found", menu_gamecount);
    VDP_drawText(buffer, 1, 1);

    sprintf(buffer, "Page %d on %d", page + 1, pageCount);
    VDP_drawText(buffer, 1, 2);

    for(u16 i = startIndex; i <= endIndex; i++) {
        printGameName(i);
    }

    u8 lastY = 4 + 2 * (endIndex % GAMES_PER_PAGE);
    u8 nextY = lastY + 2;

    u8 nextX = (page == 0) ? 1 : 13;

    VDP_setTextPalette(2);
    if (page > 0) {
        VDP_drawText("< Previous", 1, nextY);
    }
    if (page < pageCount - 1) {
        VDP_drawText("Next >", nextX, nextY);
    }
}

static void printGameName(u16 position) {
    VDP_setTextPalette(selectedGame == position ? 2 : 3);

    char name[GAME_ENTRY_SIZE+1];
    memcpy(name, (const char *)(menu_gamelist + position * GAME_ENTRY_SIZE), GAME_ENTRY_SIZE);
    name[GAME_ENTRY_SIZE] = 0; // null-terminate

    VDP_drawText(name, 2, 4 + 2 * (position % GAMES_PER_PAGE));
}
