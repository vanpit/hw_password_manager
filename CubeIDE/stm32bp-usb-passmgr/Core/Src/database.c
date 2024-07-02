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
#include "string.h"
#include "stm32f4xx_hal.h"
#include "crc.h"

static uint32_t dataStartAddr = 0x08060000;
static uint32_t backupStartAddr = 0x08040000;
static uint32_t optionsStartAddr = 0x08020000;

void GetDBSignature(uint8_t *sigBuffer, int nSlots, uint8_t key, uint8_t dbmode) {
	uint32_t readAddr = dataStartAddr;
	if (dbmode == PM_DATABASE_MODE_BACKUP) readAddr = backupStartAddr;
	else if (dbmode == PM_DATABASE_MODE_MAIN) readAddr = dataStartAddr;
	else return;

	Flash_Read_Data(readAddr+(sizeof(PasswordSlot)*nSlots), (uint32_t*)sigBuffer, 1);
	for (int i = 0; i < 4; i++ ) sigBuffer[i] ^= key;
}

bool GetPasswordSlot(PasswordSlot *slotBuffer, int nSlot, uint8_t key, uint8_t dbmode)
{
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
	uint32_t retDataCode = Flash_Write_Data(dataStartAddr, (uint32_t*)slotBuffer, (sizeof(PasswordSlot)/4) * nSlots + 1);
	uint32_t retBackupCode = Flash_Write_Data(backupStartAddr, (uint32_t*)slotBuffer, (sizeof(PasswordSlot)/4) * nSlots + 1);
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
