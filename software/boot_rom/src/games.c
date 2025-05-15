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
    u16 endIndex = pageLastIndex;
    if (pageLastIndex > lastIndex) {
        endIndex = lastIndex;
    }

    char buffer[32];
    sprintf(buffer, "%d games found", menu_gamecount);
    VDP_setTextPalette(2);
    VDP_drawText(buffer, 1, 1);


    for(u16 i = startIndex; i <= endIndex; i++) {
        printGameName(i);
    }
}

static void printGameName(u16 position) {
    VDP_setTextPalette(selectedGame == position ? 2 : 3);

    char name[GAME_ENTRY_SIZE+1];
    memcpy(name, (const char *)(menu_gamelist + position * GAME_ENTRY_SIZE), GAME_ENTRY_SIZE);
    name[GAME_ENTRY_SIZE] = 0; // null-terminate

    VDP_drawText(name, 2, 5 + 2 * (position % GAMES_PER_PAGE));
}
