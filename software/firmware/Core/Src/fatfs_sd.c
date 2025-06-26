#define TRUE  1
#define FALSE 0
#define bool BYTE

#define DEFAULT_SD_READY_TIMEOUT 500
#define DEFAULT_SD_BUSY_TIMEOUT_AFTER_WRITE 250 // Timeout de 250 ms
#define DEFAULT_SD_BUSY_TIMEOUT_AFTER_WRITE_MULTI 500

#include "stm32f4xx_hal.h"

#include "diskio.h"
#include "fatfs_sd.h"
#include <string.h>
#include <stdbool.h>
#include "log_uart.h"

static uint8_t SD_ReadyWait_Timeout(uint16_t timeout_ms);

uint16_t Timer1, Timer2;					/* 1ms Timer Counter */

static volatile DSTATUS Stat = STA_NOINIT;	/* Disk Status */
static uint8_t CardType;                    /* Type 0:MMC, 1:SDC, 2:Block addressing */
static uint8_t PowerFlag = 0;				/* Power flag */

// Flag pour la synchronisation DMA
volatile HAL_StatusTypeDef spiDmaTransferStatus = HAL_OK; // Pour stocker le statut du transfert DMA
volatile bool spiDmaTransferComplete = false;         // Mis à true par les callbacks DMA

/***************************************
 * SPI functions
 **************************************/

/* slave select */
static void SELECT(void)
{
    HAL_GPIO_WritePin(SD_CS_PORT, SD_CS_PIN, GPIO_PIN_RESET);
}

/* slave deselect */
static void DESELECT(void)
{
    HAL_GPIO_WritePin(SD_CS_PORT, SD_CS_PIN, GPIO_PIN_SET);
}

/* SPI transmit a byte */
static void SPI_TxByte(uint8_t data)
{
    while(!__HAL_SPI_GET_FLAG(HSPI_SDCARD, SPI_FLAG_TXE));
    HAL_SPI_Transmit(HSPI_SDCARD, &data, 1, SPI_TIMEOUT);
}

/* SPI receive a byte */
static uint8_t SPI_RxByte(void)
{
    uint8_t dummy, data;
    dummy = 0xFF;

    while(!__HAL_SPI_GET_FLAG(HSPI_SDCARD, SPI_FLAG_TXE));
    HAL_SPI_TransmitReceive(HSPI_SDCARD, &dummy, &data, 1, SPI_TIMEOUT);

    return data;
}

/***************************************
 * SD functions
 **************************************/

/* wait SD ready */
static uint8_t SD_ReadyWait(void)
{
    return SD_ReadyWait_Timeout(500);
}

/* power on */
static void SD_PowerOn(void)
{
    DESELECT();
    for (int i = 0; i < 10; i++) SPI_TxByte(0xFF);
    DESELECT();
    SPI_RxByte(); // un dernier dummy clock pour être sûr
    PowerFlag = 1;
}

/* power off */
static void SD_PowerOff(void) 
{
    PowerFlag = 0;
}

/* check power flag */
static uint8_t SD_CheckPower(void) 
{
    return PowerFlag;
}

/* receive data block */
static bool SD_RxDataBlock(BYTE *buff, UINT len)
{
    uint8_t token;
    HAL_StatusTypeDef dma_status_check; // Pour vérifier le statut de lancement du DMA

    /* 1. Attente du Start Token (0xFE) - Inchangé */
    Timer1 = 200; // Timeout de 200ms pour le token
    uint32_t loop_count = 0;
    do {
        token = SPI_RxByte(); // Utilise ta fonction SPI_RxByte existante (qui logue ses propres erreurs HAL si besoin)
//        if (token != 0xFF) {
//            logUart("SD_RxDataBlock: Token candidate: 0x%02X (iter: %lu, T1: %lu)", token, loop_count, Timer1);
//        }
        loop_count++;
    } while((token == 0xFF) && Timer1);

    if(token != 0xFE) {
        logUart("SD_RxDataBlock: Echec Start Token! Final Token: 0x%02X (iter: %lu, T1: %lu)", token, loop_count, Timer1);
        return FALSE;
    }
    // Si on est ici, le token 0xFE a été reçu.
//    logUart("SD_RxDataBlock: Start Token 0xFE OK! (iter: %lu, T1: %lu)", loop_count, Timer1);

    /* 2. Réception du bloc de données de 'len' (512) octets en utilisant DMA */
    spiDmaTransferComplete = false; // Réinitialise le flag avant de lancer le DMA
    spiDmaTransferStatus = HAL_BUSY;  // État initial en attente

    // Pour recevoir des données, on doit aussi envoyer des octets "dummy" (0xFF).
    // Remplis ton buffer 'buff' avec 0xFF s'il ne l'est pas déjà, car 'buff' sera utilisé pour TX et RX.
    // Ou utilise un buffer TX séparé rempli de 0xFF si 'buff' ne doit pas être modifié avant réception.
    // Pour HAL_SPI_TransmitReceive_DMA, pTxData et pRxData peuvent pointer vers le même buffer.
    memset(buff, 0xFF, len); // Prépare les octets dummy à envoyer

    // Lance le transfert SPI en mode DMA
    // Assure-toi que HSPI_SDCARD est bien ton handle SPI (ex: &hspi1)
    dma_status_check = HAL_SPI_TransmitReceive_DMA(HSPI_SDCARD, buff, buff, (uint16_t)len);

    if (dma_status_check != HAL_OK) {
        // Le lancement du DMA a échoué (ex: DMA occupé, mauvais paramètres)
        logUart("SD_RxDataBlock: HAL_SPI_TransmitReceive_DMA launch FAILED! Status: %d", dma_status_check);
        // HSPI_SDCARD->ErrorCode pourrait aussi avoir des infos ici
        return FALSE;
    }

//    logUart("SD_RxDataBlock: DMA Transfer for %u bytes started...", len);

    // Attente de la fin du transfert DMA (avec un timeout)
    // Timer1 peut être réutilisé ou un nouveau timer/compteur peut être utilisé.
    // Disons un timeout de 100ms pour le transfert DMA du bloc.
    Timer1 = 100; // Timeout de 100ms pour le transfert DMA
    while (!spiDmaTransferComplete && Timer1) {
        // Attente active. Dans un système plus complexe, on pourrait passer à une autre tâche.
        // Assure-toi que Timer1 est bien décrémenté par ton ISR SysTick.
    }

    if (!spiDmaTransferComplete) { // Timeout DMA
        logUart("SD_RxDataBlock: DMA Transfer TIMEOUT! (Timer1 expired)");
        // Il faut essayer d'avorter le transfert DMA en cours
        HAL_SPI_DMAStop(HSPI_SDCARD);
        return FALSE;
    }

    if (spiDmaTransferStatus != HAL_OK) { // Erreur reportée par le callback DMA/SPI
        logUart("SD_RxDataBlock: DMA Transfer FAILED! Reported Status: %d, SPI Error: 0x%lX", spiDmaTransferStatus, HSPI_SDCARD.ErrorCode);
        return FALSE;
    }

//    logUart("SD_RxDataBlock: DMA Transfer OK!");

    /* 3. Rejeter les 2 octets de CRC - Inchangé */
    SPI_RxByte(); // Ces appels utilisent toujours le mode polling/bloquant pour 1 octet
    SPI_RxByte();

    return TRUE; // Succès
}

/* transmit data block */
#if _USE_WRITE == 1
static bool SD_TxDataBlock(const uint8_t *buff, BYTE token)
{
    /* wait SD ready */
    if (SD_ReadyWait_Timeout(DEFAULT_SD_READY_TIMEOUT) != 0xFF) { // DEFAULT_SD_READY_TIMEOUT à définir
        logUart("SD_TxDataBlock: SD_ReadyWait_Timeout FAILED avant token");
        return FALSE;
    }

    /* transmit token */
    SPI_TxByte(token);

    /* if it's not STOP token, transmit data */
    if (token == 0xFE || token == 0xFC) { // Si c'est un token de début de bloc de données
        HAL_StatusTypeDef dma_launch_status;

        spiDmaTransferComplete = false;     // Réinitialise le flag
        spiDmaTransferStatus = HAL_BUSY; // État initial

        // Lancement du transfert DMA pour les 512 octets de données
        // buff est const, mais HAL_SPI_Transmit_DMA attend uint8_t* non-const.
        // Un cast est nécessaire si buff est vraiment const.
        dma_launch_status = HAL_SPI_Transmit_DMA(HSPI_SDCARD, (uint8_t*)buff, 512);

        if (dma_launch_status != HAL_OK) {
            logUart("SD_TxDataBlock: HAL_SPI_Transmit_DMA launch FAILED! Status: %d", dma_launch_status);
            // Tenter d'arrêter le DMA si le lancement a échoué mais que le handle est actif
            // HAL_SPI_DMAStop(HSPI_SDCARD); // Peut ne pas être nécessaire si le lancement échoue proprement
            return FALSE;
        }
//        logUart("SD_TxDataBlock: DMA TX de 512 octets demarre...");

        // Attente de la fin du transfert DMA (avec timeout Timer1)
        Timer1 = 100; // Timeout de 100ms pour le transfert DMA
        while(!spiDmaTransferComplete && Timer1) {
            // Attente active ou permettre à d'autres tâches de s'exécuter si RTOS
        }

        if (!spiDmaTransferComplete) { // Timeout DMA
            logUart("SD_TxDataBlock: DMA TX TIMEOUT! (Timer1 a expire)");
            HAL_SPI_DMAStop(HSPI_SDCARD); // Important d'arrêter le DMA en cas de timeout
            return FALSE;
        }

        if (spiDmaTransferStatus != HAL_OK) { // Erreur DMA reportée par callback SPI
            logUart("SD_TxDataBlock: DMA TX FAILED! Status: %d, SPI Error: 0x%lX", spiDmaTransferStatus, HSPI_SDCARD.ErrorCode);
            // Le DMA a pu être stoppé dans le ErrorCallback, ou le stopper ici aussi
            // HAL_SPI_DMAStop(HSPI_SDCARD);
            return FALSE;
        }
//        logUart("SD_TxDataBlock: DMA TX de 512 octets OK!");

        // Après le DMA des 512 octets de données, envoyer les 2 octets de CRC (en mode polling)
        SPI_TxByte(0xFF); // CRC Fictif octet 1
        SPI_TxByte(0xFF); // CRC Fictif octet 2

        // Maintenant, attendre et recevoir le "Data Response Token" de la carte
        Timer1 = 200; // Timeout de 200ms pour la réponse (N_WR)
        uint8_t data_resp_token;
        do {
            data_resp_token = SPI_RxByte(); // SPI_RxByte reste en polling
        } while (data_resp_token == 0xFF && Timer1); // La carte envoie 0xFF avant sa réponse

        if (!Timer1 || (data_resp_token & 0x1F) != 0x05) { // 0bXXX00101 = Data Accepted
            logUart("SD_TxDataBlock: Erreur Data Response Token ou Timeout. Token: 0x%02X, T1: %lu", data_resp_token, Timer1);
            SD_ReadyWait_Timeout(DEFAULT_SD_BUSY_TIMEOUT_AFTER_WRITE); // Tenter de laisser la carte finir
            return FALSE;
        }
//        logUart("SD_TxDataBlock: Data Response Token OK (0x%02X)", data_resp_token);

        // Données acceptées, attendre que la carte ne soit plus "busy" (écriture physique)
        if (SD_ReadyWait_Timeout(DEFAULT_SD_BUSY_TIMEOUT_AFTER_WRITE) != 0xFF) { // Mettre un timeout long (ex: 500ms)
            logUart("SD_TxDataBlock: Timeout attente fin de BUSY apres ecriture bloc.");
            return FALSE;
        }
//        logUart("SD_TxDataBlock: Carte plus busy apres ecriture bloc.");
        return TRUE;

    } else if (token == 0xFD) { // C'est un token STOP_TRAN (après un CMD25)
        // Après avoir envoyé 0xFD, la carte SD a besoin que le maître lise au moins un octet "stuff byte".
        SPI_RxByte(); // Lire et ignorer cet octet.

        // Ensuite, la carte passe "busy" pour écrire tous les blocs précédents. Attendre la fin.
        if (SD_ReadyWait_Timeout(DEFAULT_SD_BUSY_TIMEOUT_AFTER_WRITE_MULTI) != 0xFF) { // Timeout très long ici
            logUart("SD_TxDataBlock: Timeout attente fin de BUSY apres STOP_TRAN (0xFD).");
            return FALSE;
        }
//        logUart("SD_TxDataBlock: Carte plus busy apres STOP_TRAN.");
        return TRUE;
    }

    logUart("SD_TxDataBlock: Token inconnu ou logique non implementee: 0x%02X", token);
    return FALSE;
}
#endif /* _USE_WRITE */

// Fonction helper pour SD_ReadyWait avec timeout (tu en as peut-être déjà une)
// Retourne 0xFF si la carte est prête, ou le dernier octet lu si timeout.
static uint8_t SD_ReadyWait_Timeout(uint16_t timeout_ms) {
    uint8_t res;
    Timer2 = timeout_ms; // Utilise Timer2, comme SD_ReadyWait()
    do {
        res = SPI_RxByte();
    } while (res != 0xFF && Timer2); // Vérifie Timer2
    if (res != 0xFF && Timer2 == 0) { // Pour être explicite sur la condition de timeout
        logUart("SD_ReadyWait_Timeout: TIMEOUT! Dernier octet lu: 0x%02X", res);
    }
    return res;
}

/* transmit command */
static BYTE SD_SendCmd(BYTE cmd, uint32_t arg)
{
    uint8_t n, res, crc;

    // 1. Toujours relâcher la SD avant d'envoyer une commande
    DESELECT();
    SPI_RxByte(); // 1 dummy clock

    // 2. Sélectionner la SD et attendre 1 dummy clock
    SELECT();
    SPI_RxByte();

    // 3. Envoyer la commande
    SPI_TxByte(cmd);
    SPI_TxByte((uint8_t)(arg >> 24));
    SPI_TxByte((uint8_t)(arg >> 16));
    SPI_TxByte((uint8_t)(arg >> 8));
    SPI_TxByte((uint8_t)arg);

    crc = 0x01; // Default dummy CRC
    if (cmd == CMD0) crc = 0x95;
    if (cmd == CMD8) crc = 0x87;
    SPI_TxByte(crc);

    // 4. Pour CMD12, skip un octet
    if (cmd == CMD12) SPI_RxByte();

    // 5. Attendre la réponse (bit 7 à 0)
    n = 20;
    do {
        res = SPI_RxByte();
    } while ((res & 0x80) && --n);

    // 6. NE PAS DESELECT ici si tu attends un transfert de données juste après !

    return res;
}

/***************************************
 * user_diskio.c functions
 **************************************/

/* initialize SD */
DSTATUS SD_disk_initialize(BYTE drv) 
{
    uint8_t n, type, ocr[4];

    /* single drive, drv should be 0 */
    if(drv) return STA_NOINIT;

    /* no disk */
    if(Stat & STA_NODISK) return Stat;

    if (HAL_SPI_DeInit(HSPI_SDCARD) != HAL_OK) {
        logUart("HAL_SPI_DeInit (low speed) failed");
        SD_PowerOff();
        return STA_NOINIT;
    }
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256; // Pour <= 400kHz
    if (HAL_SPI_Init(HSPI_SDCARD) != HAL_OK) {
        logUart("HAL_SPI_Init (low speed) failed");
        SD_PowerOff();
        return STA_NOINIT;
    }
    logUart("SPI Set to Low Speed for SD Init");

    /* power on */
    SD_PowerOn();

    /* check disk type */
    type = 0;

    /* send GO_IDLE_STATE command */
    if (SD_SendCmd(CMD0, 0) == 1)
    {
        /* timeout 1 sec */
        Timer1 = 1000;

        /* SDC V2+ accept CMD8 command, http://elm-chan.org/docs/mmc/mmc_e.html */
        if (SD_SendCmd(CMD8, 0x1AA) == 1)
        {
            /* operation condition register */
            for (n = 0; n < 4; n++)
            {
                ocr[n] = SPI_RxByte();
            }

            /* voltage range 2.7-3.6V */
            if (ocr[2] == 0x01 && ocr[3] == 0xAA)
            {
                /* ACMD41 with HCS bit */
                do {
                    if (SD_SendCmd(CMD55, 0) <= 1 && SD_SendCmd(CMD41, 1UL << 30) == 0) break;
                } while (Timer1);

                /* READ_OCR */
                if (Timer1 && SD_SendCmd(CMD58, 0) == 0)
                {
                    /* Check CCS bit */
                    for (n = 0; n < 4; n++)
                    {
                        ocr[n] = SPI_RxByte();
                    }

                    /* SDv2 (HC or SC) */
                    type = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
                }
            }
        }
        else
        {
            /* SDC V1 or MMC */
            type = (SD_SendCmd(CMD55, 0) <= 1 && SD_SendCmd(CMD41, 0) <= 1) ? CT_SD1 : CT_MMC;

            do
            {
                if (type == CT_SD1)
                {
                    if (SD_SendCmd(CMD55, 0) <= 1 && SD_SendCmd(CMD41, 0) == 0) break; /* ACMD41 */
                }
                else
                {
                    if (SD_SendCmd(CMD1, 0) == 0) break; /* CMD1 */
                }

            } while (Timer1);

            /* SET_BLOCKLEN */
            if (!Timer1 || SD_SendCmd(CMD16, 512) != 0) type = 0;
        }
    }

    CardType = type;

    /* Clear STA_NOINIT */
    if (type)
    {
        logUart("SD Init Success, CardType: 0x%02X", CardType);
        if (HAL_SPI_DeInit(HSPI_SDCARD) != HAL_OK) {
            logUart("HAL_SPI_DeInit (high speed) failed");
            SD_PowerOff();
            return STA_NOINIT;
        }
        // Configure ici la vitesse de travail que tu souhaites tester
        // Par exemple SPI_BAUDRATEPRESCALER_4 pour 21MHz
        // Ou SPI_BAUDRATEPRESCALER_8 pour 10.5MHz
        hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8; // <--- VITESSE DE TRAVAIL

        if (HAL_SPI_Init(HSPI_SDCARD) != HAL_OK) {
            logUart("HAL_SPI_Init (high speed %s) failed", (hspi1.Init.BaudRatePrescaler == SPI_BAUDRATEPRESCALER_4) ? "21MHz" : "10.5MHz");
            SD_PowerOff();
            return STA_NOINIT;
        }
        logUart("SPI Set to High Speed (%s) for Data Transfer", (hspi1.Init.BaudRatePrescaler == SPI_BAUDRATEPRESCALER_4) ? "21MHz" : "10.5MHz");
        Stat &= ~STA_NOINIT; // Initialisation réussie
    }
    else
    {
        logUart("SD Card Initialization Failed (type == 0 at the end)");
        /* Initialization failed */
        SD_PowerOff();
    }

    return Stat;
}

/* return disk status */
DSTATUS SD_disk_status(BYTE drv) 
{
    if (drv) return STA_NOINIT;
    return Stat;
}

/* read sector */
DRESULT SD_disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count) 
{
    /* pdrv should be 0 */
    if (pdrv || !count) return RES_PARERR;

    /* no disk */
    if (Stat & STA_NOINIT) return RES_NOTRDY;

    /* convert to byte address */
    if (!(CardType & CT_SD2)) sector *= 512;

    SELECT();

    if (count == 1)
    {
        /* READ_SINGLE_BLOCK */
        if ((SD_SendCmd(CMD17, sector) == 0) && SD_RxDataBlock(buff, 512)) count = 0;
    }
    else
    {
        /* READ_MULTIPLE_BLOCK */
        if (SD_SendCmd(CMD18, sector) == 0)
        {
            do {
                if (!SD_RxDataBlock(buff, 512)) break;
                buff += 512;
            } while (--count);

            /* STOP_TRANSMISSION */
            SD_SendCmd(CMD12, 0);
        }
    }

    /* Idle */
    DESELECT();
    SPI_RxByte();

    return count ? RES_ERROR : RES_OK;
}

/* write sector */
#if _USE_WRITE == 1
DRESULT SD_disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count) 
{
    /* pdrv should be 0 */
    if (pdrv || !count) return RES_PARERR;

    /* no disk */
    if (Stat & STA_NOINIT) return RES_NOTRDY;

    /* write protection */
    if (Stat & STA_PROTECT) return RES_WRPRT;

    /* convert to byte address */
    if (!(CardType & CT_SD2)) sector *= 512;

    SELECT();

    if (count == 1)
    {
        /* WRITE_BLOCK */
        if ((SD_SendCmd(CMD24, sector) == 0) && SD_TxDataBlock(buff, 0xFE))
            count = 0;
    }
    else
    {
        /* WRITE_MULTIPLE_BLOCK */
        if (CardType & CT_SD1)
        {
            SD_SendCmd(CMD55, 0);
            SD_SendCmd(CMD23, count); /* ACMD23 */
        }

        if (SD_SendCmd(CMD25, sector) == 0)
        {
            do {
                if(!SD_TxDataBlock(buff, 0xFC)) break;
                buff += 512;
            } while (--count);

            /* STOP_TRAN token */
            if(!SD_TxDataBlock(0, 0xFD))
            {
                count = 1;
            }
        }
    }

    /* Idle */
    DESELECT();
    SPI_RxByte();

    return count ? RES_ERROR : RES_OK;
}
#endif /* _USE_WRITE */

/* ioctl */
DRESULT SD_disk_ioctl(BYTE drv, BYTE ctrl, void *buff) 
{
    DRESULT res;
    uint8_t n, csd[16], *ptr = buff;
    WORD csize;

    /* pdrv should be 0 */
    if (drv) return RES_PARERR;
    res = RES_ERROR;

    if (ctrl == CTRL_POWER)
    {
        switch (*ptr)
        {
        case 0:
            SD_PowerOff();		/* Power Off */
            res = RES_OK;
            break;
        case 1:
            SD_PowerOn();		/* Power On */
            res = RES_OK;
            break;
        case 2:
            *(ptr + 1) = SD_CheckPower();
            res = RES_OK;		/* Power Check */
            break;
        default:
            res = RES_PARERR;
        }
    }
    else
    {
        /* no disk */
        if (Stat & STA_NOINIT) return RES_NOTRDY;

        SELECT();

        switch (ctrl)
        {
        case GET_SECTOR_COUNT:
            /* SEND_CSD */
            if ((SD_SendCmd(CMD9, 0) == 0) && SD_RxDataBlock(csd, 16))
            {
                if ((csd[0] >> 6) == 1)
                {
                    /* SDC V2 */
                    csize = csd[9] + ((WORD) csd[8] << 8) + 1;
                    *(DWORD*) buff = (DWORD) csize << 10;
                }
                else
                {
                    /* MMC or SDC V1 */
                    n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
                    csize = (csd[8] >> 6) + ((WORD) csd[7] << 2) + ((WORD) (csd[6] & 3) << 10) + 1;
                    *(DWORD*) buff = (DWORD) csize << (n - 9);
                }
                res = RES_OK;
            }
            break;
        case GET_SECTOR_SIZE:
            *(WORD*) buff = 512;
            res = RES_OK;
            break;
        case CTRL_SYNC:
            if (SD_ReadyWait() == 0xFF) res = RES_OK;
            break;
        case MMC_GET_CSD:
            /* SEND_CSD */
            if (SD_SendCmd(CMD9, 0) == 0 && SD_RxDataBlock(ptr, 16)) res = RES_OK;
            break;
        case MMC_GET_CID:
            /* SEND_CID */
            if (SD_SendCmd(CMD10, 0) == 0 && SD_RxDataBlock(ptr, 16)) res = RES_OK;
            break;
        case MMC_GET_OCR:
            /* READ_OCR */
            if (SD_SendCmd(CMD58, 0) == 0)
            {
                for (n = 0; n < 4; n++)
                {
                    *ptr++ = SPI_RxByte();
                }
                res = RES_OK;
            }
        default:
            res = RES_PARERR;
        }

        DESELECT();
        SPI_RxByte();
    }

    return res;
}
