/* Test the Reed-Solomon codecs
 * for various block sizes and with random data and random error patterns
 *
 * Copyright 2002 Phil Karn, KA9Q
 * May be used under the terms of the GNU Lesser General Public License (LGPL)
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include "include/reed_solomon.h"

int exercise_int(void);

word_t error_offsets[RS_MAX_ERRORS];
word_t error_values[RS_MAX_ERRORS];

int main()
{
	srandom(time(NULL));

	printf("Testing (%d,%d) . . .\n", TOTAL_SYMBOL_COUNT, DATA_SYMBOL_COUNT);

	word_t block[SYMBOL_TABLE_WORDS + 1], tblock[SYMBOL_TABLE_WORDS + 1];
	int i;
	int errlocs[TOTAL_SYMBOL_COUNT + 1];
	int errors;
	int derrors;
	symbol_t errval;
	int errloc;

	/* Test up to the error correction capacity of the code */
	for (errors = 0; errors <= RS_MAX_SYMBOL_ERRORS; errors++) {
		/* Load block with random data and encode */
		for (i = 0; i < PAYLOAD_DATA_WORDS; i++) {
			block[i] = random();
		}

		encode_rs(block);

		/* Make temp copy, seed with errors */
		memcpy(tblock, block, sizeof(block));
		memset(errlocs, 0, sizeof(errlocs));

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
		derrors = decode_rs(tblock, error_offsets, error_values);

		printf("\n\n(%d,%d) decoder says %d errors, actual number is %d\n", TOTAL_SYMBOL_COUNT, DATA_SYMBOL_COUNT, derrors, errors);

		if (memcmp(tblock, block, sizeof(tblock)) != 0) {
			for (i = 0; i < TOTAL_SYMBOL_COUNT; i++) {
				if (symbol_get(tblock, i) ^ symbol_get(block, i)) {
					printf("(%d,%d) error at %d\n", TOTAL_SYMBOL_COUNT, DATA_SYMBOL_COUNT, i);
				}
			}
			printf("\n");
		}
	}
	return 0;
}
