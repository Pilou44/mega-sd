#ifndef _MENU_MEMORY_H_
#define _MENU_MEMORY_H_

#include <genesis.h>

#define MENU_LIST_ADDR 0xFF2000
#define MENU_MAX_GAMES 512
#define GAME_ENTRY_SIZE 32

#define menu_gamecount (*(volatile u16*)MENU_LIST_ADDR)
#define menu_gameselected (*(volatile u16*)MENU_LIST_ADDR + 2)
#define menu_gamelist  ((volatile u8*)(MENU_LIST_ADDR + 4))

#endif // _MEMORY_H_
