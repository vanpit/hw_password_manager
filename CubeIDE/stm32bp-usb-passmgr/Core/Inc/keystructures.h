/*
 * keystructures.h
 *
 *  Created on: Sep 24, 2023
 *      Author: piotr
 */

#include "usb_hid_keys.h"

#ifndef INC_KEYSTRUCTURES_H_
#define INC_KEYSTRUCTURES_H_



#endif /* INC_KEYSTRUCTURES_H_ */

#define SEQ_RELEASE 0x00
#define SEQ_TAB 0x01
#define SEQ_ENTER 0x02
#define SEQ_CAPSLOCK 0x03

typedef struct KbdReport {
	uint8_t modifier;
	uint8_t reserved;
	uint8_t key[6];
} KbdReport;

typedef struct ConversionTableEntry {
	char letter;
	KbdReport report;
} ConversionTableEntry;

uint8_t* GetReportByChar(char);
