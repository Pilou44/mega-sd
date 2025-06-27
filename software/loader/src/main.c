#include <genesis.h>
#include "res_gfx.h"
#include "res_sprite.h"

u16 palette[64];


// player (sonic) sprite
Sprite* player;

// void showLoader();
void PLAYER_init();

int main(bool hardReset) {
    
    // initialization
    VDP_setScreenWidth320();
    // set all palette to black
    PAL_setColors(0, (u16*) palette_black, 64, CPU);
    // init sprite engine with default parameters
    SPR_init();

    // showLoader();
    PLAYER_init();
    
    // update sprite
    SPR_update();
    // and init map
    SYS_doVBlankProcess();

    
    memcpy(&palette[0], palette_all.data, 64 * 2);
    
    // fade in
    PAL_fadeIn(0, (4 * 16) - 1, palette, 20, TRUE);

    while(TRUE)
    {
        // first
        // handleInput();

        // if (!paused)
        // {
        //     // update player first
        //     PLAYER_update();
        //     // then set camera from player position
        //     CAMERA_centerOn(F32_toInt(posX), F32_toInt(posY));

        //     // then we can update entities
        //     ENTITIES_update();

        //     // better to do it separately, when camera position is up to date
        //     PLAYER_updateScreenPosition();
        //     ENTITIES_updateScreenPosition();
        // }

        // update sprites
        SPR_update();

        // sync frame and do vblank process
        SYS_doVBlankProcess();
    }

    // // release maps
    // MEM_free(bga);
    // MEM_free(bgb);

    return 0;
}

// void showLoader() {
//     // prepare palettes (BGB image contains the 4 palettes data)
//     memcpy(&palette[0], palette_all.data, 64 * 2);
//     // PAL_setColors(0, (u16 *)palette_grey, 16, DMA);

//     // VDP_setTextPalette(3);

//     VDP_drawText("Loading", 10, 10);
// }

// u16 PLAYER_init(u16 vramIndex)
void PLAYER_init()
{
    // default speeds
    // maxSpeed = MAX_SPEED_DEFAULT;
    // jumpSpeed = JUMP_SPEED_DEFAULT;
    // gravity = GRAVITY_DEFAULT;

    // set main sprite position (camera position may be adjusted depending it)
    // posX = FIX32(48L);
    // posY = MAX_POSY;
    // movX = FIX32(0);
    // movY = FIX32(0);
    // xOrder = 0;
    // yOrder = 0;

    // init sonic sprite
    player = SPR_addSprite(&sonic_sprite, 0, 0, TILE_ATTR(PAL0, TRUE, FALSE, FALSE));

    // do not used static vram allocation here
    // return vramIndex;
}
