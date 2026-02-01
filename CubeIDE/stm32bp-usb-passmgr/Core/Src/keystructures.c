/*
 * keystructures.c
 *
 *  Created on: Sep 24, 2023
 *      Author: piotr
 */

#include <stdint.h>
#include "keystructures.h"

static ConversionTableEntry conversionTable[] = {
		{0x00, {0, 0, {0, 0, 0, 0, 0, 0}}},
		{0xFF, {0, 0, {0, 0, 0, 0, 0, 0}}},
		{0x01, {0, 0, {KEY_TAB, 0, 0, 0, 0, 0}}},
		{0x02, {0, 0, {KEY_ENTER, 0, 0, 0, 0, 0}}},
		{0x03, {0, 0, {KEY_CAPSLOCK, 0, 0, 0, 0, 0}}},
		{'A', {KEY_MOD_LSHIFT, 0, {KEY_A, 0, 0, 0, 0, 0}}},
		{'B', {KEY_MOD_LSHIFT, 0, {KEY_B, 0, 0, 0, 0, 0}}},
		{'C', {KEY_MOD_LSHIFT, 0, {KEY_C, 0, 0, 0, 0, 0}}},
		{'D', {KEY_MOD_LSHIFT, 0, {KEY_D, 0, 0, 0, 0, 0}}},
		{'E', {KEY_MOD_LSHIFT, 0, {KEY_E, 0, 0, 0, 0, 0}}},
		{'F', {KEY_MOD_LSHIFT, 0, {KEY_F, 0, 0, 0, 0, 0}}},
		{'G', {KEY_MOD_LSHIFT, 0, {KEY_G, 0, 0, 0, 0, 0}}},
		{'H', {KEY_MOD_LSHIFT, 0, {KEY_H, 0, 0, 0, 0, 0}}},
		{'I', {KEY_MOD_LSHIFT, 0, {KEY_I, 0, 0, 0, 0, 0}}},
		{'J', {KEY_MOD_LSHIFT, 0, {KEY_J, 0, 0, 0, 0, 0}}},
		{'K', {KEY_MOD_LSHIFT, 0, {KEY_K, 0, 0, 0, 0, 0}}},
		{'L', {KEY_MOD_LSHIFT, 0, {KEY_L, 0, 0, 0, 0, 0}}},
		{'M', {KEY_MOD_LSHIFT, 0, {KEY_M, 0, 0, 0, 0, 0}}},
		{'N', {KEY_MOD_LSHIFT, 0, {KEY_N, 0, 0, 0, 0, 0}}},
		{'O', {KEY_MOD_LSHIFT, 0, {KEY_O, 0, 0, 0, 0, 0}}},
		{'P', {KEY_MOD_LSHIFT, 0, {KEY_P, 0, 0, 0, 0, 0}}},
		{'Q', {KEY_MOD_LSHIFT, 0, {KEY_Q, 0, 0, 0, 0, 0}}},
		{'R', {KEY_MOD_LSHIFT, 0, {KEY_R, 0, 0, 0, 0, 0}}},
		{'S', {KEY_MOD_LSHIFT, 0, {KEY_S, 0, 0, 0, 0, 0}}},
		{'T', {KEY_MOD_LSHIFT, 0, {KEY_T, 0, 0, 0, 0, 0}}},
		{'U', {KEY_MOD_LSHIFT, 0, {KEY_U, 0, 0, 0, 0, 0}}},
		{'V', {KEY_MOD_LSHIFT, 0, {KEY_V, 0, 0, 0, 0, 0}}},
		{'W', {KEY_MOD_LSHIFT, 0, {KEY_W, 0, 0, 0, 0, 0}}},
		{'X', {KEY_MOD_LSHIFT, 0, {KEY_X, 0, 0, 0, 0, 0}}},
		{'Y', {KEY_MOD_LSHIFT, 0, {KEY_Y, 0, 0, 0, 0, 0}}},
		{'Z', {KEY_MOD_LSHIFT, 0, {KEY_Z, 0, 0, 0, 0, 0}}},
		{'a', {0, 0, {KEY_A, 0, 0, 0, 0, 0}}},
		{'b', {0, 0, {KEY_B, 0, 0, 0, 0, 0}}},
		{'c', {0, 0, {KEY_C, 0, 0, 0, 0, 0}}},
		{'d', {0, 0, {KEY_D, 0, 0, 0, 0, 0}}},
		{'e', {0, 0, {KEY_E, 0, 0, 0, 0, 0}}},
		{'f', {0, 0, {KEY_F, 0, 0, 0, 0, 0}}},
		{'g', {0, 0, {KEY_G, 0, 0, 0, 0, 0}}},
		{'h', {0, 0, {KEY_H, 0, 0, 0, 0, 0}}},
		{'i', {0, 0, {KEY_I, 0, 0, 0, 0, 0}}},
		{'j', {0, 0, {KEY_J, 0, 0, 0, 0, 0}}},
		{'k', {0, 0, {KEY_K, 0, 0, 0, 0, 0}}},
		{'l', {0, 0, {KEY_L, 0, 0, 0, 0, 0}}},
		{'m', {0, 0, {KEY_M, 0, 0, 0, 0, 0}}},
		{'n', {0, 0, {KEY_N, 0, 0, 0, 0, 0}}},
		{'o', {0, 0, {KEY_O, 0, 0, 0, 0, 0}}},
		{'p', {0, 0, {KEY_P, 0, 0, 0, 0, 0}}},
		{'q', {0, 0, {KEY_Q, 0, 0, 0, 0, 0}}},
		{'r', {0, 0, {KEY_R, 0, 0, 0, 0, 0}}},
		{'s', {0, 0, {KEY_S, 0, 0, 0, 0, 0}}},
		{'t', {0, 0, {KEY_T, 0, 0, 0, 0, 0}}},
		{'u', {0, 0, {KEY_U, 0, 0, 0, 0, 0}}},
		{'v', {0, 0, {KEY_V, 0, 0, 0, 0, 0}}},
		{'w', {0, 0, {KEY_W, 0, 0, 0, 0, 0}}},
		{'x', {0, 0, {KEY_X, 0, 0, 0, 0, 0}}},
		{'y', {0, 0, {KEY_Y, 0, 0, 0, 0, 0}}},
		{'z', {0, 0, {KEY_Z, 0, 0, 0, 0, 0}}},
		{'1', {0, 0, {KEY_1, 0, 0, 0, 0, 0}}},
		{'2', {0, 0, {KEY_2, 0, 0, 0, 0, 0}}},
		{'3', {0, 0, {KEY_3, 0, 0, 0, 0, 0}}},
		{'4', {0, 0, {KEY_4, 0, 0, 0, 0, 0}}},
		{'5', {0, 0, {KEY_5, 0, 0, 0, 0, 0}}},
		{'6', {0, 0, {KEY_6, 0, 0, 0, 0, 0}}},
		{'7', {0, 0, {KEY_7, 0, 0, 0, 0, 0}}},
		{'8', {0, 0, {KEY_8, 0, 0, 0, 0, 0}}},
		{'9', {0, 0, {KEY_9, 0, 0, 0, 0, 0}}},
		{'0', {0, 0, {KEY_0, 0, 0, 0, 0, 0}}},
		{'!', {KEY_MOD_LSHIFT, 0, {KEY_1, 0, 0, 0, 0, 0}}},
		{'@', {KEY_MOD_LSHIFT, 0, {KEY_2, 0, 0, 0, 0, 0}}},
		{'#', {KEY_MOD_LSHIFT, 0, {KEY_3, 0, 0, 0, 0, 0}}},
		{'$', {KEY_MOD_LSHIFT, 0, {KEY_4, 0, 0, 0, 0, 0}}},
		{'%', {KEY_MOD_LSHIFT, 0, {KEY_5, 0, 0, 0, 0, 0}}},
		{'^', {KEY_MOD_LSHIFT, 0, {KEY_6, 0, 0, 0, 0, 0}}},
		{'&', {KEY_MOD_LSHIFT, 0, {KEY_7, 0, 0, 0, 0, 0}}},
		{'*', {KEY_MOD_LSHIFT, 0, {KEY_8, 0, 0, 0, 0, 0}}},
		{'(', {KEY_MOD_LSHIFT, 0, {KEY_9, 0, 0, 0, 0, 0}}},
		{')', {KEY_MOD_LSHIFT, 0, {KEY_0, 0, 0, 0, 0, 0}}},
		{'_', {KEY_MOD_LSHIFT, 0, {KEY_MINUS, 0, 0, 0, 0, 0}}},
		{'+', {KEY_MOD_LSHIFT, 0, {KEY_EQUAL, 0, 0, 0, 0, 0}}},
		{'-', {0, 0, {KEY_MINUS, 0, 0, 0, 0, 0}}},
		{'=', {0, 0, {KEY_EQUAL, 0, 0, 0, 0, 0}}},
		{'.', {0, 0, {KEY_DOT, 0, 0, 0, 0, 0}}}
};

uint8_t* GetReportByChar(char chr)
{
	for (int i = 0; i< sizeof(conversionTable); i++) {
		if (conversionTable[i].letter == chr) return (uint8_t*)&(conversionTable[i].report);
	}
	return (uint8_t*)&(conversionTable[0].report);
}
