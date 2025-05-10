#include "splash.h"
#include "splash_res.h"


u16 palette[64];

void fadeIn(s16 palet, u16 numFrame, bool async);

void showSplash()
{
    volatile u16 *menu_comm = (u16 *)MENU_COMM_ADDR;
    *menu_comm = 0;

    Sprite *star;
    u16 ind;

    u16 bgBaseTileIndex[2];
    Map *bgb;

    memcpy(&palette[0], palette_splash.data, 16 * 2);
    memcpy(&palette[16], palette_splash.data, 16 * 2);

    PAL_setColors(0, (u16 *)palette_black, 64, DMA);
    PAL_setColors(32, palette_sprite.data, 16, DMA);

    VDP_setScreenWidth320();

    SPR_init();

    // load background tilesets in VRAM
    ind = TILE_USER_INDEX;
    bgBaseTileIndex[0] = ind;
    VDP_loadTileSet(&bgb_tileset, ind, DMA);
    ind += bgb_tileset.numTile;

    bgb = MAP_create(&bgb_map, BG_B, TILE_ATTR_FULL(0, FALSE, FALSE, FALSE, bgBaseTileIndex[0]));
    s16 camX = 0;
    MAP_scrollTo(bgb, camX, 0);

    VDP_setTextPalette(1);
    VDP_drawText("Wechant Loup presents", 9, 22);
    VDP_setHorizontalScroll(BG_A, 4);

    star = SPR_addSprite(&sprite_star, 168, 91, TILE_ATTR(PAL2, TRUE, FALSE, FALSE));
    SPR_setVisibility(star, HIDDEN);
    SPR_update();

    fadeIn(0, 20, FALSE);

    while (camX < 320)
    {
        camX += 4;
        MAP_scrollTo(bgb, camX, 0);
        SPR_update();
        SYS_doVBlankProcess();
    }
    fadeIn(1, 20, TRUE);
    SPR_setVisibility(star, VISIBLE);

    while (*menu_comm == 0)
    {
        SPR_update();
        SYS_doVBlankProcess();
    }
    SPR_setVisibility(star, HIDDEN);
    PAL_fadeOut(16, (2 * 16) - 1, 20, TRUE);
    while (camX < 640)
    {
        camX += 4;
        MAP_scrollTo(bgb, camX, 0);
        SPR_update();
        SYS_doVBlankProcess();
    }

    VDP_clearText(9, 22, 21);
    SPR_releaseSprite(star);
    MEM_free(bgb);
}

void fadeIn(s16 palet, u16 numFrame, bool async)
{
    s16 startIndex = palet * 16;
    s16 endIndex = ((palet + 1) * 16) - 1;
    PAL_fadeIn(startIndex, endIndex, &palette[startIndex], numFrame, async);
}
