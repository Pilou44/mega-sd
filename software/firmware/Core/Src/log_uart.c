/*
 * log_uart.c
 *
 *  Created on: May 22, 2025
 *      Author: guill
 */

#include "log_uart.h"


// Redirige printf vers l’UART2
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

void logUart(const char *format, ...) {
    char buffer[128];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    printf("%s\r\n", buffer);
}
