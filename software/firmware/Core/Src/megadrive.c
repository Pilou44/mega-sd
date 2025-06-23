/*
 * megadrive.c
 *
 *  Created on: May 24, 2025
 *      Author: Guillaume Beguin
 */

#include "megadrive.h"
#include "fatfs.h"
#include "log_uart.h"
#include <string.h>

int loadRom(const char *path);
uint16_t getRomWord(uint32_t addr);
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
    if (portD_val & (1UL << 10)) { address |= (1 << 0);  } // A01 (PD10)
    if (portD_val & (1UL << 12)) { address |= (1 << 1);  } // A02 (PD12)
    if (portD_val & (1UL << 14)) { address |= (1 << 2);  } // A03 (PD14)
    // A04 est sur PC8
    // A05 est sur PC9
    if (portD_val & (1UL << 0))  { address |= (1 << 5);  } // A06 (PD0)
    if (portD_val & (1UL << 7))  { address |= (1 << 6);  } // A07 (PD7)
    if (portD_val & (1UL << 9))  { address |= (1 << 7);  } // A08 (PD9)
    if (portD_val & (1UL << 3))  { address |= (1 << 8);  } // A09 (PD3)
    if (portD_val & (1UL << 5))  { address |= (1 << 9);  } // A10 (PD5)
    if (portD_val & (1UL << 2))  { address |= (1 << 10); } // A11 (PD2)
    if (portD_val & (1UL << 6))  { address |= (1 << 11); } // A12 (PD6)
    // A13 est sur PC11
    // A14 est sur PC7
    if (portD_val & (1UL << 15)) { address |= (1 << 14); } // A15 (PD15)
    if (portD_val & (1UL << 13)) { address |= (1 << 15); } // A16 (PD13)
    if (portD_val & (1UL << 11)) { address |= (1 << 16); } // A17 (PD11)
    if (portD_val & (1UL << 4))  { address |= (1 << 17); } // A18 (PD4)
    if (portD_val & (1UL << 1))  { address |= (1 << 18); } // A19 (PD1)
    // A20 est sur PC12
    // A21 est sur PC10
    // A22 est sur PC13
    // A23 est sur PC6

    // Extraire les bits de GPIOC et les placer dans 'address'
    if (portC_val & (1UL << 8))  { address |= (1 << 3);  } // A04 (PC8)
    if (portC_val & (1UL << 9))  { address |= (1 << 4);  } // A05 (PC9)
    if (portC_val & (1UL << 11)) { address |= (1 << 12); } // A13 (PC11)
    if (portC_val & (1UL << 7))  { address |= (1 << 13); } // A14 (PC7)
    if (portC_val & (1UL << 12)) { address |= (1 << 19); } // A20 (PC12)
    if (portC_val & (1UL << 10)) { address |= (1 << 20); } // A21 (PC10)
    if (portC_val & (1UL << 13)) { address |= (1 << 21); } // A22 (PC13)
    if (portC_val & (1UL << 6))  { address |= (1 << 22); } // A23 (PC6)

    return address;
}

void writeData(uint16_t dataWord) {
    uint16_t portEOutputPattern = 0;

    // Mapping des bits de dataWord (D00-D15) vers les broches de GPIOE
    // selon V2_specs.md :cite[fil_vomozH7Y1lvpqA]
    if (dataWord & 0x0001) { portEOutputPattern |= (1 << 14); } // D00 -> PE14
    if (dataWord & 0x0002) { portEOutputPattern |= (1 << 8);  } // D01 -> PE8
    if (dataWord & 0x0004) { portEOutputPattern |= (1 << 4);  } // D02 -> PE4
    if (dataWord & 0x0008) { portEOutputPattern |= (1 << 3);  } // D03 -> PE3
    if (dataWord & 0x0010) { portEOutputPattern |= (1 << 2);  } // D04 -> PE2
    if (dataWord & 0x0020) { portEOutputPattern |= (1 << 1);  } // D05 -> PE1
    if (dataWord & 0x0040) { portEOutputPattern |= (1 << 11); } // D06 -> PE11
    if (dataWord & 0x0080) { portEOutputPattern |= (1 << 15); } // D07 -> PE15
    if (dataWord & 0x0100) { portEOutputPattern |= (1 << 13); } // D08 -> PE13
    if (dataWord & 0x0200) { portEOutputPattern |= (1 << 7);  } // D09 -> PE7
    if (dataWord & 0x0400) { portEOutputPattern |= (1 << 5);  } // D10 -> PE5
    if (dataWord & 0x0800) { portEOutputPattern |= (1 << 0);  } // D11 -> PE0
    if (dataWord & 0x1000) { portEOutputPattern |= (1 << 6);  } // D12 -> PE6
    if (dataWord & 0x2000) { portEOutputPattern |= (1 << 9);  } // D13 -> PE9
    if (dataWord & 0x4000) { portEOutputPattern |= (1 << 10); } // D14 -> PE10
    if (dataWord & 0x8000) { portEOutputPattern |= (1 << 12); } // D15 -> PE12

    GPIOE->ODR = portEOutputPattern;
}

void assertDtack(void) {
    // Signaler à la MD que les données sont prêtes (PA1 BAS)
    // Le bit 1 est mis à 0 en écrivant 1 sur le bit (1+16) = 17 de BSRR.
    GPIOA->BSRR = (1UL << (1 + 16)); // Met pa1 à 0
}

void deassertDtack(void) {
    // Mettre PA1 (/DTACK) au niveau HAUT (1)
    // Pour mettre à 1 avec BSRR, on écrit dans les bits 0-15.
    GPIOA->BSRR = (1UL << 1); // Met pa1 à 1
}

void maintainDtackFixDuration(void) {
	// Délai pour l'étape 9 (environ 25 NOPs pour ~150ns @ 168MHz, à ajuster)
	__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); // 5
	__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); // 10
	__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); // 15
	__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); // 20
	__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); // 25
}

void maintainDtackWithAS(void) {
	// Attendre que /AS (PC5) repasse à HAUT
	while ((GPIOC->IDR & (1UL << 5)) == 0) {
	    // Boucle tant que /AS est BAS
	    // Attention : si /AS reste bloqué BAS pour une raison X, cette boucle est infinie.
	    // Pour un test, c'est ok. Pour un produit, un timeout serait bien.
	}
}

void enableDataBusOutput(void) {
    // S'assurer que la direction est STM32 -> MD (si ce n'est pas déjà fait/fixe)
    // Pour rappel, PA11 (dirData) HAUT pour STM32->MD
    // Si PA11 est géré dynamiquement (pas le cas pour l'instant) :
    // GPIOA->BSRR = (1UL << 11); // Set pa11 HIGH. UL pour unsigned long, bonne pratique.

    // Activer les buffers de données en mettant PA10 (nOeData) à BAS (0)
    // Pour mettre à 0 avec BSRR, on écrit dans les bits 16-31.
    // Le bit 10 est mis à 0 en écrivant 1 sur le bit (10+16) = 26 de BSRR.
    GPIOA->BSRR = (1UL << (10 + 16)); // Met pa10 à 0
}

void disableDataBusOutput(void) {
    // Désactiver les buffers de données en mettant PA10 (nOeData) à HAUT (1)
    // Pour mettre à 1 avec BSRR, on écrit dans les bits 0-15.
    GPIOA->BSRR = (1UL << 10); // Met pa10 à 1
}

bool isChipEnableLow(void) {
    // Lire l'état de la broche PC4.
    // Le registre IDR (Input Data Register) de GPIOC contient l'état des broches.
    // Si le bit 4 de GPIOC->IDR est à 0, alors PC4 est BAS.
    if ((GPIOC->IDR & (1UL << 4)) == 0) {
        return true; // PC4 est au niveau BAS
    } else {
        return false; // PC4 est au niveau HAUT
    }
}

bool isChipEnableHigh(void) {
    return !isChipEnableLow();
}

bool isReadCycle(void) {
    // Lire l'état de PC0 (/UWR) et PC2 (/LWR)
    // Rappel : ils sont actifs BAS. Pour une lecture, ils doivent être HAUT.

    bool uwrIsHigh = (GPIOC->IDR & (1UL << 0)) != 0; // Vérifie si PC0 est HAUT
    bool lwrIsHigh = (GPIOC->IDR & (1UL << 2)) != 0; // Vérifie si PC2 est HAUT

    if (uwrIsHigh && lwrIsHigh) {
        return true; // C'est un cycle de lecture (les deux sont inactifs/HAUT)
    } else {
        return false; // C'est un cycle d'écriture (au moins un des deux est actif/BAS)
    }
}



// Ouvre la ROM et prépare la lecture
int loadRom(const char *path) {
    FRESULT res;
    res = f_open(&rom_file, path, FA_READ);
    if (res != FR_OK) {
        logUart("Erreur ouverture ROM: %d", res);
        return 0;
    }
    rom_size = f_size(&rom_file);
    // Charge le premier buffer
    f_lseek(&rom_file, 0);
    UINT br;
    res = f_read(&rom_file, rom_buffer, ROM_BUFFER_SIZE, &br);
    buffer_addr_start = 0;
    logUart("ROM chargee, taille = %lu octets", rom_size);
    return 1;
}

// Fonction pour fournir le mot demandé par la Mega Drive
uint16_t getRomWord(uint32_t addr) {
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
// 1) Détecter /CEO (Chip Enable) au niveau BAS.
// 2) Vérifier que /LWR et /UWR (Write Strobes) sont au niveau HAUT (c'est une lecture).
// 3) Lire l'adresse (A1-A23) depuis les broches d'adresse.
// 4) Récupérer le mot de donnée (16 bits) correspondant à cette adresse depuis la carte SD.
// 5) Positionner DIR_Data (PA11) au niveau HAUT (direction STM32 -> MD). --> Inutile pour le moment, on ne fait que de la lecture. On peut laisser PA11 au niveau haut en permanence
// 6) Écrire le mot de donnée sur les broches de données (Port E).
// 7) Positionner /OE_Data (PA10) au niveau BAS (activer la sortie des buffers de données).
// 8) Positionner /DTACK (PA1) au niveau BAS (signaler "donnée prête").
// 9) Attendre une courte période (pour la durée de /DTACK).
// 10) Positionner /DTACK (PA1) au niveau HAUT.
// 11) Positionner /OE_Data (PA10) au niveau HAUT (désactiver la sortie des buffers de données).
// 12) Attendre que /CEO remonte au niveau HAUT (fin du cycle MD)
void main_megadrive_loop(void) {
    uint32_t access_count = 0;
    while (1) {
        // Détecte un accès Mega Drive (exemple : polling sur /ROM actif bas)
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_5) == GPIO_PIN_RESET) {
            // Récupère l'adresse (19 bits)
            uint32_t addr = (GPIOB->IDR & 0xFFFF) | ((GPIOC->IDR & 0x7) << 16);

            // Sur port d'extension, A0 est toujours 0, donc addr doit être pair
            addr &= ~1;

            uint16_t data = getRomWord(addr);
            // Place la donnée sur le bus data (port D)
            GPIOD->ODR = data;

            if (++access_count % 10000 == 0) {
                logUart("Acces Mega Drive #%lu, addr=0x%06lX", access_count, addr);
            }

            // (Optionnel) Attendre la fin du cycle /ROM ou synchroniser le timing
        }
        // (Optionnel) Ajoute ici des hooks UART/debug
    }
}

void boot(void) {
	// 1. Ouvre la ROM
	if (!loadRom("boot/Sonic.md")) {
		logUart("Echec ouverture ROM boot/Sonic.md");
		while(1); // Stoppe tout si erreur
	}

	logUart("En attente Mega Drive...");

	// 2. Boucle principale, scrute /ROM
	main_megadrive_loop();
}
