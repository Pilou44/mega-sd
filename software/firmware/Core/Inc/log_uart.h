/*
 * log_uart.h
 *
 *  Created on: May 22, 2025
 *      Author: guill
 */

#ifndef INC_LOG_UART_H_
#define INC_LOG_UART_H_

#include <stdio.h>
#include <stdarg.h>
#include "usart.h"

void log_uart(const char *format, ...);

#endif /* INC_LOG_UART_H_ */
