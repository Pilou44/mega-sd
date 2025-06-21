/*
 * megadrive.h
 *
 *  Created on: May 24, 2025
 *      Author: Guillaume Beguin
 */

#ifndef INC_MEGADRIVE_H_
#define INC_MEGADRIVE_H_

#include <stdint.h>

uint32_t readAddress(void);
void writeData(uint16_t);
void enableDataBusOutput(void);
void disableDataBusOutput(void);


void megadrive_boot(void);

#endif /* INC_MEGADRIVE_H_ */
