/*
 * database.h
 *
 *  Created on: Sep 24, 2023
 *      Author: piotr
 */

#ifndef INC_DATABASE_H_
#define INC_DATABASE_H_

#define PM_DATABASE_MODE_MAIN 0x01
#define PM_DATABASE_MODE_BACKUP 0x02

#include <stdbool.h>

typedef struct PasswordSlot {
	char name[25];
	char username[25];
	char password[25];
	char previouspassword[25];
	uint32_t checksum;
} PasswordSlot;

typedef struct OptionsSlot {
	uint8_t options[4];
} OptionsSlot;

void GetDBSignature(uint8_t*, int, uint8_t, uint8_t);
bool GetPasswordSlot(PasswordSlot*, int, uint8_t, uint8_t);
bool WritePasswordSlots(uint8_t*, int);
void SlotAddChecksum(PasswordSlot*);
void GetOptions(uint8_t*);
void SaveOptions(uint8_t*);


#endif /* INC_DATABASE_H_ */

