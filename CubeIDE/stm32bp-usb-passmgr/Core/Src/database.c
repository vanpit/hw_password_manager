/*
 * database.c
 *
 *  Created on: Sep 24, 2023
 *      Author: piotr
 */

#include <stdint.h>
#include <stdbool.h>
#include "database.h"
#include "FLASH_SECTOR_F4.h"
#include "stm32f4xx_hal.h"
#include "crc.h"

#ifndef __CDT_PARSER__
#define STATIC_ASSERT(cond, name) typedef char static_assert_##name[(cond) ? 1 : -1]
STATIC_ASSERT((sizeof(PasswordSlot) % 4) == 0, passwordslot_size_must_be_multiple_of_4);
#endif

static uint32_t dataStartAddr = 0x08060000;
static uint32_t backupStartAddr = 0x08040000;
static uint32_t optionsStartAddr = 0x08020000;

void GetDBSignature(uint8_t *sigBuffer, int nSlots, uint8_t key, uint8_t dbmode)
{
	if (nSlots <= 0 || nSlots > NUMBER_OF_SLOTS) return;

	uint32_t readAddr = dataStartAddr;
	if (dbmode == PM_DATABASE_MODE_BACKUP) readAddr = backupStartAddr;
	else if (dbmode == PM_DATABASE_MODE_MAIN) readAddr = dataStartAddr;
	else return;

	Flash_Read_Data(readAddr+(sizeof(PasswordSlot)*nSlots), (uint32_t*)sigBuffer, 1);
	for (int i = 0; i < 4; i++ ) sigBuffer[i] ^= key;
}

bool GetPasswordSlot(PasswordSlot *slotBuffer, int nSlot, uint8_t key, uint8_t dbmode)
{
	if (nSlot < 0 || nSlot >= NUMBER_OF_SLOTS) return false;

	uint32_t readAddr = dataStartAddr;
	if (dbmode == PM_DATABASE_MODE_BACKUP) readAddr = backupStartAddr;
	else if (dbmode == PM_DATABASE_MODE_MAIN) readAddr = dataStartAddr;
	else return false;

	Flash_Read_Data(readAddr+(sizeof(PasswordSlot)*nSlot), (uint32_t*)slotBuffer, (sizeof(PasswordSlot)/4));
	for (int i = 0; i < sizeof(PasswordSlot); i++ ) ((uint8_t*)slotBuffer)[i] ^= key;

	uint32_t checksum = HAL_CRC_Calculate(&hcrc, (uint32_t*)slotBuffer, ((sizeof(PasswordSlot))/4)-1);
	if (checksum == slotBuffer->checksum) return true;
	else return false;
}

bool WritePasswordSlots(uint8_t *slotBuffer, int nSlots)
{
	if (nSlots <= 0 || nSlots > NUMBER_OF_SLOTS) return false;

	uint32_t retDataCode = Flash_Write_Data(dataStartAddr, (uint32_t*)slotBuffer, (sizeof(PasswordSlot)/4) * nSlots + DATABASE_SIGNATURE_SIZE_IN_32_T);
	uint32_t retBackupCode = Flash_Write_Data(backupStartAddr, (uint32_t*)slotBuffer, (sizeof(PasswordSlot)/4) * nSlots + DATABASE_SIGNATURE_SIZE_IN_32_T);
	if ((retDataCode > 0) || (retBackupCode > 0)) return false;
	else return true;
}

void SlotAddChecksum(PasswordSlot *slot) {
	slot->checksum = HAL_CRC_Calculate(&hcrc, (uint32_t*)slot, ((sizeof(PasswordSlot))/4)-1);
}

void GetOptions(uint8_t* optBuffer)
{
	Flash_Read_Data(optionsStartAddr, (uint32_t*)optBuffer, 1);
}

void SaveOptions(uint8_t* optBuffer)
{
	Flash_Write_Data(optionsStartAddr, (uint32_t*)optBuffer, 1);
}
