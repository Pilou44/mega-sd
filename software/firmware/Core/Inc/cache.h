/*
 * cache.h
 *
 *  Created on: Jun 25, 2025
 *      Author: Guillaume Beguin
 */

#ifndef INC_CACHE_H_
#define INC_CACHE_H_

#define CACHE_PAGE_COUNT 3
#define CACHE_PAGE_SIZE 32768

// Optionnel: Assertion statique pour la taille totale (si C11+)
#if __STDC_VERSION__ >= 201112L
_Static_assert((CACHE_PAGE_COUNT * CACHE_PAGE_SIZE) <= (96 * 1024), "Cache size exceeds 96KB allowance");
#endif

#include <stdint.h>
#include <stdbool.h>

// Structure pour une entrée (page) de cache
typedef struct {
    bool      isValid;                  // Ce slot de cache contient-il des données valides ?
    uint32_t  romBlockAddress;          // Adresse de début du bloc de ROM de CACHE_PAGE_SIZE stocké ici
                                        // (ex: 0x00000, 0x08000, 0x10000, etc.)
    uint32_t  lastAccessTimestamp;      // Pour la stratégie de remplacement LRU
    uint8_t   buffer[CACHE_PAGE_SIZE];  // Le buffer RAM réel pour cette page
} CachePageEntry_t;


// Prototypes des fonctions de gestion du cache (à implémenter dans cache.c)

/**
 * @brief Initialise le module de cache.
 * Doit être appelée une fois au démarrage.
 */
void cache_init(void);

/**
 * @brief Charge une ROM depuis son path
 */
int loadRom(const char *path);

/**
 * @brief Récupère un mot de 16 bits depuis la ROM via le cache.
 * Gère les cache hits et les cache misses (chargement depuis la SD).
 * @param romByteAddress Adresse en OCTETS dans la ROM du premier octet du mot à lire.
 * @return Le mot de 16 bits lu (Big Endian converti en Little Endian natif),
 *         ou une valeur d'erreur (ex: 0xFFFF) si l'adresse est invalide ou si une erreur de lecture SD se produit.
 */
uint16_t cache_getRomWord(uint32_t romByteAddress);

// D'autres fonctions pourraient être utiles, par exemple pour invalider le cache,
// mais ne sont pas strictement nécessaires pour une lecture seule.

#endif /* INC_CACHE_H_ */
