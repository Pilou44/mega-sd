/*
 * main.c
 *
 *  Created on: 7 avr. 2022
 *      Author: guill
 */

#include <genesis.h>
#include "splash.h"
#include "title.h"
#include "level.h"


int main(u16 hard)
{
    showSplash();

    while (TRUE)
    {
        u8 players = showTitle();
        showLevel(players);
    }

    return 0;
}
