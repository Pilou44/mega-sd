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



uint32_t readAddress(void) {
    uint32_t address = 0;
    uint32_t portD_val;
    uint32_t portC_val;

    // Lire la valeur complète des ports d'entrée C et D
    // L'accès direct aux registres IDR est le plus rapide.
    // La syntaxe exacte peut dépendre de tes définitions de registres (CMSIS standard).
    portD_val = GPIOD->IDR;
    portC_val = GPIOC->IDR;

    // Extraire les bits de GPIOD et les placer dans 'address'
    if (portD_val & (1 << 10)) { address |= (1 << 0);  } // A01 (PD10)
    if (portD_val & (1 << 12)) { address |= (1 << 1);  } // A02 (PD12)
    if (portD_val & (1 << 14)) { address |= (1 << 2);  } // A03 (PD14)
    // A04 est sur PC8
    // A05 est sur PC9
    if (portD_val & (1 << 0))  { address |= (1 << 5);  } // A06 (PD0)
    if (portD_val & (1 << 7))  { address |= (1 << 6);  } // A07 (PD7)
    if (portD_val & (1 << 9))  { address |= (1 << 7);  } // A08 (PD9)
    if (portD_val & (1 << 3))  { address |= (1 << 8);  } // A09 (PD3)
    if (portD_val & (1 << 5))  { address |= (1 << 9);  } // A10 (PD5)
    if (portD_val & (1 << 2))  { address |= (1 << 10); } // A11 (PD2)
    if (portD_val & (1 << 6))  { address |= (1 << 11); } // A12 (PD6)
    // A13 est sur PC11
    // A14 est sur PC7
    if (portD_val & (1 << 15)) { address |= (1 << 14); } // A15 (PD15)
    if (portD_val & (1 << 13)) { address |= (1 << 15); } // A16 (PD13)
    if (portD_val & (1 << 11)) { address |= (1 << 16); } // A17 (PD11)
    if (portD_val & (1 << 4))  { address |= (1 << 17); } // A18 (PD4)
    if (portD_val & (1 << 1))  { address |= (1 << 18); } // A19 (PD1)
    // A20 est sur PC12
    // A21 est sur PC10
    // A22 est sur PC13
    // A23 est sur PC6

    // Extraire les bits de GPIOC et les placer dans 'address'
    if (portC_val & (1 << 8))  { address |= (1 << 3);  } // A04 (PC8)
    if (portC_val & (1 << 9))  { address |= (1 << 4);  } // A05 (PC9)
    if (portC_val & (1 << 11)) { address |= (1 << 12); } // A13 (PC11)
    if (portC_val & (1 << 7))  { address |= (1 << 13); } // A14 (PC7)
    if (portC_val & (1 << 12)) { address |= (1 << 19); } // A20 (PC12)
    if (portC_val & (1 << 10)) { address |= (1 << 20); } // A21 (PC10)
    if (portC_val & (1 << 13)) { address |= (1 << 21); } // A22 (PC13)
    if (portC_val & (1 << 6))  { address |= (1 << 22); } // A23 (PC6)

    return address;
}



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
    uint32_t access_count = 0;
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

            if (++access_count % 10000 == 0) {
                log_uart("Acces Mega Drive #%lu, addr=0x%06lX", access_count, addr);
            }

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
