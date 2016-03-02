
#include <stdint.h>

typedef uint16_t symbol_t; /* the lower BITS_PER_SYMBOL bits are valid */
typedef uint16_t word_t;
typedef uint32_t dword_t;

#define RS_MAX_SYMBOL_ERRORS 8 		/* Tolerate up to 8 symbol errors per block */
#define RS_MAX_ERRORS		(RS_MAX_SYMBOL_ERRORS * 2)
#define BITS_PER_SYMBOL		13		/* DON'T CHANGE THIS! Current code is dependent in this value */
#define GFPOLY 				0x201B
#define FCS 1
#define PRIM 1
#define IPRIM 1

#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define TOTAL_SYMBOL_COUNT	((1 << BITS_PER_SYMBOL) - 1) /* 0x1FFF (8192 - 1) symbols per block */
#define PARITY_SYMBOL_COUNT	(RS_MAX_SYMBOL_ERRORS * 2)
#define DATA_SYMBOL_COUNT	(TOTAL_SYMBOL_COUNT - PARITY_SYMBOL_COUNT)
#define BITS_PER_WORD		(sizeof(word_t) * 8)
#define SYMBOL_TABLE_WORDS	(TOTAL_SYMBOL_COUNT * BITS_PER_SYMBOL / BITS_PER_WORD + 1)

/* Aliases for legibility */
#define SYMBOL_MASK			TOTAL_SYMBOL_COUNT
#define A0					TOTAL_SYMBOL_COUNT

#define PAYLOAD_DATA_WORDS	(DATA_SYMBOL_COUNT * BITS_PER_SYMBOL / BITS_PER_WORD)
#define PAYLOAD_DATA_BYTES	(PAYLOAD_DATA_WORDS * 2)

