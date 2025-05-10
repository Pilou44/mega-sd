#ifndef _MENU_MEMORY_H_
#define _MENU_MEMORY_H_

#include <genesis.h>

#define MENU_LIST_ADDR 0xFF2000
#define MENU_MAX_GAMES 512
#define GAME_ENTRY_SIZE 32

// Compteur du nombre de jeux
//#define MENU_GAMECOUNT_ADDR MENU_LIST_ADDR
// Début de la table de jeux (juste après le compteur)
//#define MENU_GAMELIST_ADDR (MENU_LIST_ADDR + 2)

//extern volatile u16 *menu_gamecount;
//extern volatile u8 *menu_gamelist;

#define menu_gamecount (*(volatile u16*)MENU_LIST_ADDR)
#define menu_gamelist  ((volatile u8*)(MENU_LIST_ADDR + 2))
#define menu_comm (*(volatile u16*)0xFF1506)

#endif // _MEMORY_H_
