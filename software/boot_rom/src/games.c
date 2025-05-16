#include "games.h"
#include "menu_memory.h"

#define GAMES_PER_PAGE 10

#define NEXT_PAGE -1
#define PREVIOUS_PAGE -2

s32 selectedGame;
u8 page;
u16 startIndex;
u16 endIndex;
u8 pageCount;

static void displayGamePage();
static void printGameName(u16 position);
static void joyEvent(u16 joy, u16 changed, u16 state);
static void clearAllTexts();

void showGames() {
    page = 0;
    selectedGame = 0;

    
    PAL_setColors(0, (u16 *)palette_red, 16, DMA);
    PAL_setColors(16, (u16 *)palette_black, 16, DMA);
    PAL_setColors(32, (u16 *)palette_green, 16, DMA);
    PAL_setColors(48, (u16 *)palette_blue, 16, DMA);

    JOY_init();
    JOY_setEventHandler(joyEvent);

    
    while (TRUE) {
        displayGamePage();
        SYS_doVBlankProcess();
    }
}

static void displayGamePage() {
    startIndex = page * GAMES_PER_PAGE;
    u16 lastIndex = menu_gamecount - 1;
    u16 pageLastIndex = (page +1) * GAMES_PER_PAGE - 1;
    endIndex = (pageLastIndex > lastIndex) ? lastIndex : pageLastIndex;
    pageCount = lastIndex / GAMES_PER_PAGE + 1;

    if (selectedGame >= 0 && (selectedGame < startIndex || selectedGame > endIndex)) {
        selectedGame = startIndex;
    } else if (selectedGame == PREVIOUS_PAGE && page == 0) {
        selectedGame = NEXT_PAGE;
    } else if (selectedGame == NEXT_PAGE && page == pageCount - 1) {
        selectedGame = PREVIOUS_PAGE;
    }

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

    if (page > 0) {
        VDP_setTextPalette(selectedGame == PREVIOUS_PAGE ? 2 : 3);
        VDP_drawText("< Previous", 1, nextY);
    }
    if (page < pageCount - 1) {
        VDP_setTextPalette(selectedGame == NEXT_PAGE ? 2 : 3);
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

static void clearAllTexts() {
    VDP_clearTextArea(0, 0, 60, 25);
}

static void joyEvent(u16 joy, u16 changed, u16 state) {
    if (changed & state & BUTTON_START) {
        if (selectedGame >= startIndex && selectedGame <= endIndex) {
            menu_gameselected = selectedGame + 1;
        }
    } else if (changed & state & BUTTON_UP) {
        if (selectedGame > startIndex && selectedGame <= endIndex) {
            selectedGame--;
        } else if (selectedGame == NEXT_PAGE && page == 0) {
            selectedGame = endIndex;
        } else if (selectedGame == NEXT_PAGE) {
            selectedGame = PREVIOUS_PAGE;
        } else if (selectedGame == PREVIOUS_PAGE) {
            selectedGame = endIndex;
        }
    } else if (changed & state & BUTTON_DOWN) {
        if (selectedGame >= startIndex && selectedGame < endIndex) {
            selectedGame++;
        } else if (page == 0) {
            selectedGame = NEXT_PAGE;
        } else if (selectedGame == endIndex) {
            selectedGame = PREVIOUS_PAGE;
        } else if (page < pageCount - 1) {
            selectedGame = NEXT_PAGE;
        }
    } else if (changed & state & BUTTON_LEFT) {
        if (selectedGame == NEXT_PAGE && page > 0) {
            selectedGame = PREVIOUS_PAGE;
        }
    } else if (changed & state & BUTTON_RIGHT) {
        if (selectedGame == PREVIOUS_PAGE && page < pageCount - 1) {
            selectedGame = NEXT_PAGE;
        }
    } else if (changed & state & (BUTTON_A | BUTTON_B | BUTTON_C)) {
        if (selectedGame == PREVIOUS_PAGE) {
            page--;
            clearAllTexts();
        } else if (selectedGame == NEXT_PAGE) {
            page++;
            clearAllTexts();
        }
    }
}
