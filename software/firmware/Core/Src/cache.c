/*
 * cache.c
 *
 *  Created on: Jun 25, 2025
 *      Author: Guillaume Beguin
 */

#include "cache.h"
#include "fatfs.h"
#include "log_uart.h"
#include <string.h>   // Pour memset (si tu utilises HAL_SPI_TransmitReceive_DMA, tu remplis le buffer de 0xFF)

// Le pool de cache lui-même
static CachePageEntry_t cache[CACHE_PAGE_COUNT];

// Compteur global pour les timestamps LRU (Least Recently Used)
static uint32_t currentTimestamp = 0;

// Variables globales de ton fichier fatfs_sd.c qui sont maintenant gérées par le cache
FIL rom_file;
uint32_t rom_size = 0;

// (Tu devras probablement déplacer la logique de f_open, f_lseek, f_read de tes
// anciennes fonctions loadRom/getRomWord vers des fonctions helper ici, ou les appeler)


void cache_init(void) {
    logUart("Initialisation du cache (%d pages de %u Ko)...", CACHE_PAGE_COUNT, (unsigned int)(CACHE_PAGE_SIZE / 1024));
    for (int i = 0; i < CACHE_PAGE_COUNT; i++) {
        cache[i].isValid = false;
        cache[i].romBlockAddress = 0xFFFFFFFF; // Adresse invalide
        cache[i].lastAccessTimestamp = 0;
        // Le contenu de cache[i].buffer n'a pas besoin d'être initialisé ici
    }
    currentTimestamp = 0;
    // Pré-charger la première page de la ROM si rom_file est déjà ouverte et rom_size connue
    cache_getRomWord(0); // Forcerait le chargement de la première page
    logUart("Cache initialise.");
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

    // Initialiser le cache
    cache_init();

    logUart("ROM chargee, taille = %lu octets", rom_size);
    return 1;
}

// Fonction interne pour charger une page depuis la SD dans un slot de cache
// Retourne true si succès, false si erreur.
static bool load_page_from_sd(uint32_t romBlockAddrToLoad, int cacheSlotIndex) {
    // Vérifier que romBlockAddrToLoad est aligné à CACHE_PAGE_SIZE (ou l'arrondir)
    // Utiliser f_lseek pour se positionner à romBlockAddrToLoad dans rom_file
    // Utiliser f_read pour lire CACHE_PAGE_SIZE octets dans cache[cacheSlotIndex].buffer
    // Gérer les erreurs de f_lseek et f_read.
    // Mettre à jour current_buffer_valid_bytes si la lecture est partielle (fin de ROM).
    // Pour l'instant, supposons une lecture complète pour simplifier.

    FRESULT res;
    UINT bytes_read;

    logUart("Cache MISS: Chargement du bloc ROM 0x%08lX dans slot %d", romBlockAddrToLoad, cacheSlotIndex);

    // Assure-toi que rom_file est ouverte et que rom_size est connue.
    // Ces variables devront être accessibles (globales ou passées).
    // Pour cet exemple, je vais supposer qu'elles sont globales comme dans ton code précédent.
    // extern FIL rom_file; // Déclarée dans fatfs_sd.c ou équivalent
    // extern uint32_t rom_size; // Taille de la ROM active

    if (romBlockAddrToLoad >= rom_size) {
        logUart("Cache: Tentative de chargement au-dela de la taille de la ROM (0x%08lX >= 0x%08lX)", romBlockAddrToLoad, rom_size);
        return false; // Adresse invalide
    }

    // Positionnement
    res = f_lseek(&rom_file, romBlockAddrToLoad); // rom_file doit être la FIL de la ROM ouverte
    if (res != FR_OK) {
        logUart("Cache: f_lseek error %d pour adresse 0x%08lX", res, romBlockAddrToLoad);
        return false;
    }

    // Lecture
    res = f_read(&rom_file, cache[cacheSlotIndex].buffer, CACHE_PAGE_SIZE, &bytes_read);
    if (res != FR_OK) {
        logUart("Cache: f_read error %d pour adresse 0x%08lX", res, romBlockAddrToLoad);
        return false;
    }

    if (bytes_read == 0 && romBlockAddrToLoad < rom_size) {
        logUart("Cache: f_read 0 octet pour adresse 0x%08lX", romBlockAddrToLoad);
        // Cela peut arriver si romBlockAddrToLoad est valide mais qu'il n'y a plus rien à lire (fin de fichier exacte)
        // ou si le bloc demandé est entièrement après la fin de la ROM.
        // Si bytes_read < CACHE_PAGE_SIZE, le reste du buffer contiendra des données invalides,
        // mais isValid et romBlockAddress seront corrects. La lecture d'un mot devra vérifier les limites.
    }
    // Pour un cache simple, on considère que si bytes_read > 0, la page est "valide" pour ce qu'elle a lu.
    // La gestion de current_buffer_valid_bytes devient par page ou est implicite par rom_size.

    logUart("Cache: Bloc 0x%08lX charge dans slot %d (%u octets lus)", romBlockAddrToLoad, cacheSlotIndex, bytes_read);
    cache[cacheSlotIndex].isValid = true;
    cache[cacheSlotIndex].romBlockAddress = romBlockAddrToLoad;
    cache[cacheSlotIndex].lastAccessTimestamp = currentTimestamp; // Mis à jour plus tard si hit

    return true;
}


uint16_t cache_getRomWord(uint32_t romByteAddress) {
    currentTimestamp++; // Incrémente le temps global à chaque tentative d'accès

    // 1. Vérifier si l'adresse est dans les limites de la ROM
    // extern uint32_t rom_size; // Doit être accessible
    if (romByteAddress >= rom_size || romByteAddress > rom_size - 2) { // -2 pour lire un mot
        // logUart("Cache: Adresse ROM 0x%08lX hors limites (taille ROM: 0x%08lX)", romByteAddress, rom_size);
        return 0xFFFF; // Ou une autre valeur d'erreur
    }

    // 2. Calculer l'adresse de début du bloc de cache contenant cette adresse
    uint32_t targetRomBlockAddress = romByteAddress & ~(CACHE_PAGE_SIZE - 1);

    // 3. Chercher si ce bloc est déjà dans le cache (Cache Hit)
    for (int i = 0; i < CACHE_PAGE_COUNT; i++) {
        if (cache[i].isValid && cache[i].romBlockAddress == targetRomBlockAddress) {
            // Cache Hit!
            cache[i].lastAccessTimestamp = currentTimestamp; // Met à jour le timestamp LRU
            uint32_t offsetInPage = romByteAddress - targetRomBlockAddress; // ou romByteAddress % CACHE_PAGE_SIZE

            // Gérer le cas où romByteAddress + 1 dépasse la taille de la ROM
            // (Bien que le check initial romByteAddress > rom_size - 2 devrait le couvrir pour un mot)
            // if (targetRomBlockAddress + offsetInPage + 1 >= rom_size) { /* ... gérer ... */ }

            // logUart("Cache HIT: Addr 0x%08lX trouvee dans slot %d (bloc 0x%08lX)", romByteAddress, i, targetRomBlockAddress);
            return (cache[i].buffer[offsetInPage] << 8) | cache[i].buffer[offsetInPage + 1];
        }
    }

    // 4. Cache Miss: Le bloc n'est pas dans le cache. Il faut le charger.
    // Choisir une page à remplacer (stratégie LRU: la plus anciennement utilisée)
    int slotToReplace = 0;
    uint32_t oldestTimestamp = 0xFFFFFFFF;
    bool foundEmptySlot = false;

    for (int i = 0; i < CACHE_PAGE_COUNT; i++) {
        if (!cache[i].isValid) { // Privilégier un slot vide
            slotToReplace = i;
            foundEmptySlot = true;
            break;
        }
        if (cache[i].lastAccessTimestamp < oldestTimestamp) {
            oldestTimestamp = cache[i].lastAccessTimestamp;
            slotToReplace = i;
        }
    }
    // Si on a trouvé un slot vide, oldestTimestamp n'a pas été mis à jour, mais slotToReplace est correct.
    // logUart("Cache MISS: Addr 0x%08lX (bloc 0x%08lX). Remplacement du slot %d (TS: %lu)", romByteAddress, targetRomBlockAddress, slotToReplace, foundEmptySlot ? 0 : oldestTimestamp);


    // Charger la nouvelle page depuis la SD dans le slot choisi
    if (load_page_from_sd(targetRomBlockAddress, slotToReplace)) {
        cache[slotToReplace].lastAccessTimestamp = currentTimestamp; // Mettre à jour le timestamp du nouveau bloc chargé
        uint32_t offsetInPage = romByteAddress - targetRomBlockAddress;
        return (cache[slotToReplace].buffer[offsetInPage] << 8) | cache[slotToReplace].buffer[offsetInPage + 1];
    } else {
        logUart("Cache: ECHEC chargement page 0x%08lX dans slot %d", targetRomBlockAddress, slotToReplace);
        return 0xFFFF; // Erreur de lecture SD
    }
}
