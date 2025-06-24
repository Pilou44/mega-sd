/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "fatfs.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "log_uart.h"
#include "megadrive.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// Variables FatFS
FATFS fs;
FIL file;
FRESULT res;
DIR dir;
FILINFO fno;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void list_sd_root(void);
void test_file_transfer(const char *filename);
void test_file_access(const char *filename);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_FATFS_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);

  FRESULT result = f_mount(&fs, "", 1);
  if (result != FR_OK) {
      logUart("Mount failed: %d", result);
      while(1); // Stoppe tout
  }

//  boot();
//  mainMegadriveLoop();

  list_sd_root();
  test_file_transfer("test.txt");
  test_file_access("test.txt");
  test_file_transfer("roms/Sonic The Hedgehog 2 (World).md");
  test_file_access("roms/Sonic The Hedgehog 2 (World).md");
  test_file_transfer("roms/Sonic2.md");
  test_file_access("roms/Sonic2.md");
  test_file_transfer("roms/Sonic The Hedgehog (USA, Europe).md");
  test_file_access("roms/Sonic The Hedgehog (USA, Europe).md");
  test_file_transfer("roms/Sonic.md");
  test_file_access("roms/Sonic.md");
  test_file_transfer("roms/Columns (W) (REV01) [!].gen");
  test_file_access("roms/Columns (W) (REV01) [!].gen");
  test_file_transfer("roms/Columns.gen");
  test_file_access("roms/Columns.gen");
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void test_file_transfer(const char *filename) {
    FIL file;
    FRESULT res;
    UINT br;
    uint8_t buffer[32768];
    uint32_t total = 0;
    uint32_t t0, t1;

    // Ouvre le fichier (remplace le nom par celui de ta ROM)
    logUart("Test de transfert de %s avec un buffer de %d octets", filename, sizeof(buffer));
    res = f_open(&file, filename, FA_READ);
    if (res != FR_OK) {
        logUart("Open error: %d", res);
        return;
    }

    uint32_t fileSize = f_size(&file);
    logUart("Taille attendue FatFS = %lu", fileSize);

    t0 = HAL_GetTick();
    do {
        res = f_read(&file, buffer, sizeof(buffer), &br);
        total += br;
        if (res != FR_OK) {
            logUart("Read error (%s): %d", filename, res);
            break;
        }
    } while (br == sizeof(buffer));
    t1 = HAL_GetTick();

    f_close(&file);
    logUart("Lecture terminee (%s) : %ld octets lus en %ld ms", filename, total, t1 - t0);
    if (fileSize == total) {
        logUart("Transfert OK");
    } else {
        logUart("Transfert KO !!!!!!!!");
    }
}

void test_file_access(const char *filename) {
    FIL file;
    FRESULT res;
    UINT br;
    uint8_t buffer[32768];
    uint32_t t0, t1;

    // Ouvre le fichier (remplace le nom par celui de ta ROM)
    logUart("Test d'accès à %s avec un buffer de %d octets", filename, sizeof(buffer));
    res = f_open(&file, filename, FA_READ);
    if (res != FR_OK) {
        logUart("Open error: %d", res);
        return;
    }

    uint32_t fileSize = f_size(&file);

    t0 = HAL_GetTick();
    res = f_lseek(&file, 0);
    if (res != FR_OK) {
        logUart("Lseek (debut) error: %d", res);
        f_close(&file);
        return;
    }
    res = f_read(&file, buffer, sizeof(buffer), &br);
    if (res != FR_OK) {
        logUart("Read (premiers ko) error: %d", res);
        f_close(&file);
        return;
    }
    if (br == 0 && fileSize > 0) { // Si on n'a rien lu alors que le fichier n'est pas vide
        logUart("Read (premiers ko) error: 0 octet lu pour fichier non vide");
        f_close(&file);
        return;
    }
    t1 = HAL_GetTick();

    logUart("Temps de lecture pour les premiers %d ko : %ld ms", sizeof(buffer), t1 - t0);

    if (fileSize >= sizeof(buffer)) {
        t0 = HAL_GetTick();
        res = f_lseek(&file, fileSize - sizeof(buffer));
        if (res != FR_OK) {
            logUart("Lseek (fin) error: %d", res);
            f_close(&file);
            return;
        }
        res = f_read(&file, buffer, sizeof(buffer), &br);
        if (res != FR_OK) {
            logUart("Read (derniers ko) error: %d", res);
            f_close(&file);
            return;
        }
        // Pour les derniers 32Ko d'un fichier >= 32Ko, on s'attend à ce que br == sizeof(buffer)
        if (br != sizeof(buffer)) {
            logUart("Read (derniers ko) warning: n'a pas lu 32ko complets (lus: %u)", (unsigned int)br);
        }
        t1 = HAL_GetTick();

        logUart("Temps de lecture pour les derniers %d ko : %ld ms", sizeof(buffer), t1 - t0);
    } else {
        logUart("Fichier %s trop petit (%lu octets) pour lire les derniers %u ko.", filename, fileSize, (unsigned int)(sizeof(buffer) / 1024));
    }

    f_close(&file);
}

void list_sd_root(void) {
    // Monte la SD
	FRESULT result = f_mount(&fs, "", 1);
    if (result == FR_OK) {
        logUart("SD mounted");
    } else {
        logUart("Mount failed: %d", result);
        return;
    }

    // Ouvre le répertoire racine
    res = f_opendir(&dir, "/");
    if (res == FR_OK) {
        logUart("Root dir opened");
        while (1) {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0) break;
            // Affiche le nom du fichier ou du dossier
            logUart("%s", fno.fname);
        }
        f_closedir(&dir);
    } else {
        logUart("Open root failed");
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
