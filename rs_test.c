/* Test the Reed-Solomon codecs
 * for various block sizes and with random data and random error patterns
 *
 * Copyright 2002 Phil Karn, KA9Q
 * May be used under the terms of the GNU Lesser General Public License (LGPL)
 */

// #include <stm32f4xx_crc.h>
// #include <stm32f4xx_tim.h>
#include <uart.h>
#include <stdio.h>
#include <string.h>
#include <stm32f4xx_rng.h>
#include "include/reed_solomon.h"

void reed_solomon_test(void);
uint32_t random(void);
void init_RNG(void);

int main()
{
	uart_init();
	init_RNG();

	printf("Begin Reed Solomon test.\n");
	reed_solomon_test();
	printf("Reed Solomon test complete.\n");

	return 0;
};

void reed_solomon_test() {
	word_t block[SYMBOL_TABLE_WORDS + 1], tblock[SYMBOL_TABLE_WORDS + 1];
	int i;
	int errlocs[TOTAL_SYMBOL_COUNT + 1];
	int errors;
	int derrors;
	symbol_t errval;
	int errloc;
	error_marker_t corrections[RS_MAX_CORRECTIONS];
	int correction_count = 0;

	/* Test up to the error correction capacity of the code */
	for (errors = 0; errors <= RS_MAX_SYMBOL_ERRORS; errors++) {
		/* Load block with random data and encode */
		for (i = 0; i < PAYLOAD_DATA_WORDS; i++) {
			block[i] = random();
		}
		printf("encode_rs(block)\n");
		encode_rs(block);

		/* Make temp copy, seed with errors */
		memcpy(tblock, block, sizeof(block));
		memset(errlocs, 0, sizeof(errlocs));
		printf("error injection: %d\n", errors);
		for (i = 0; i < errors; i++) {
			do {
				errval = random() & SYMBOL_MASK;
			} while (errval == 0); /* Error value must be nonzero */

			do {
				errloc = random() & SYMBOL_MASK;
			} while (errlocs[errloc] != 0); /* Must not choose the same location twice */

			errlocs[errloc] = 1;

			symbol_put(tblock, errloc, symbol_get(tblock, errloc) ^ errval);
		}

		/* Decode the errored block */
		printf("decode_rs()\n");
		derrors = decode_rs(tblock, &correction_count, corrections);

		printf("(%d,%d) decoder says %d errors, true number is %d\n", TOTAL_SYMBOL_COUNT, DATA_SYMBOL_COUNT, derrors, errors);

		printf("tblock = 0x%08X\n", (unsigned int)tblock);

		for(i=0; i<correction_count; i++)
		{
			printf("0x%08X = 0x%08X\n", (unsigned int)corrections[i].pointer, (unsigned int)corrections[i].corrected_dword);
			*corrections[i].pointer = corrections[i].corrected_dword;
		}

		for (i = 0; i < TOTAL_SYMBOL_COUNT; i++) {
			if (symbol_get(tblock, i) ^ symbol_get(block, i)) {
				printf("(%d,%d) error at %d\n", TOTAL_SYMBOL_COUNT, DATA_SYMBOL_COUNT, i);
			}
		}
	}
}

/************************************************************
 * random
 */
uint32_t random(void) {
	while (RNG_GetFlagStatus(RNG_FLAG_DRDY) != SET) {
	}
	return RNG->DR;
}

/**********************************************************
 * Init Random Number generator
 */
  void init_RNG(void) {
    RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
	RNG_ClearFlag(RNG_FLAG_SECS);
	RNG_Cmd(DISABLE);
	RNG_Cmd(ENABLE);
 }
