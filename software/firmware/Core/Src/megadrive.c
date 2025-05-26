/*
 * megadrive.c
 *
 *  Created on: May 24, 2025
 *      Author: guill
 */

#include "megadrive.h"
#include "fatfs.h"
#include "log_uart.h"
#include <string.h>

int load_rom(const char *path);
uint16_t get_rom_word(uint32_t addr);
void main_megadrive_loop(void);

FIL rom_file;
uint32_t rom_size = 0;

// Pour un buffer de 32 Ko (64 secteurs, 16384 mots 16 bits)
#define ROM_BUFFER_SIZE 32768
uint8_t rom_buffer[ROM_BUFFER_SIZE];
uint32_t buffer_addr_start = 0; // Adresse de début du buffer dans la ROM

// Ouvre la ROM et prépare la lecture
int load_rom(const char *path) {
    FRESULT res;
    res = f_open(&rom_file, path, FA_READ);
    if (res != FR_OK) {
        log_uart("Erreur ouverture ROM: %d", res);
        return 0;
    }
    rom_size = f_size(&rom_file);
    // Charge le premier buffer
    f_lseek(&rom_file, 0);
    UINT br;
    res = f_read(&rom_file, rom_buffer, ROM_BUFFER_SIZE, &br);
    buffer_addr_start = 0;
    log_uart("ROM chargee, taille = %lu octets", rom_size);
    return 1;
}

// Fonction pour fournir le mot demandé par la Mega Drive
uint16_t get_rom_word(uint32_t addr) {
    // Limite à la taille de la ROM
    if (addr > rom_size - 2) return 0xFFFF;

    // Si l'adresse demandée n'est pas dans le buffer courant, recharge le buffer
    if (addr < buffer_addr_start || addr >= buffer_addr_start + ROM_BUFFER_SIZE) {
        f_lseek(&rom_file, addr & ~(ROM_BUFFER_SIZE-1));
        UINT br;
        f_read(&rom_file, rom_buffer, ROM_BUFFER_SIZE, &br);
        buffer_addr_start = addr & ~(ROM_BUFFER_SIZE-1);
    }
    uint32_t offset = addr - buffer_addr_start;
    // Les ROMs MD sont en 16 bits big endian, adapte si besoin
    return (rom_buffer[offset] << 8) | rom_buffer[offset+1];
}

// Boucle principale pour interagir avec la Mega Drive
void main_megadrive_loop(void) {
    while (1) {
        // Détecte un accès Mega Drive (exemple : polling sur /ROM actif bas)
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_5) == GPIO_PIN_RESET) {
            // Récupère l'adresse (19 bits)
            uint32_t addr = (GPIOB->IDR & 0xFFFF) | ((GPIOC->IDR & 0x7) << 16);

            // Sur port d'extension, A0 est toujours 0, donc addr doit être pair
            addr &= ~1;

            uint16_t data = get_rom_word(addr);
            // Place la donnée sur le bus data (port D)
            GPIOD->ODR = data;

            // (Optionnel) Attendre la fin du cycle /ROM ou synchroniser le timing
        }
        // (Optionnel) Ajoute ici des hooks UART/debug
    }
}

void megadrive_boot(void) {
	// 1. Ouvre la ROM
	if (!load_rom("boot/Sonic.md")) {
		log_uart("Echec ouverture ROM boot/Sonic.md");
		while(1); // Stoppe tout si erreur
	}

	log_uart("En attente Mega Drive...");

	// 2. Boucle principale, scrute /ROM
	main_megadrive_loop();
}
