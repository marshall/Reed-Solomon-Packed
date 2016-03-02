/* Stuff specific to the general (integer) version of the Reed-Solomon codecs
 *
 * Copyright 2003, Phil Karn, KA9Q
 * May be used under the terms of the GNU Lesser General Public License (LGPL)
 */
#ifndef _REED_SOLOMON_H
#define _REED_SOLOMON_H

#include <rs_types.h>

extern const word_t alpha_to[SYMBOL_TABLE_WORDS];
extern const word_t index_of[SYMBOL_TABLE_WORDS];
extern const word_t genpoly[PARITY_SYMBOL_COUNT + 1];

#define INLINE_ATTRIBUTE __attribute__((no_instrument_function, always_inline))

#define PARITY_SYMBOLS(x) (DATA_SYMBOL_COUNT +(x))
#define DIVIDE_BY_BITS_PER_WORD(x) ((x) >> 4)
#define MODULO_BITS_PER_WORD(x) ((x) - word_idx * BITS_PER_WORD)
#define GET_PARTS(i) \
	register dword_t symbits = idx * BITS_PER_SYMBOL; \
	register dword_t word_idx = DIVIDE_BY_BITS_PER_WORD(symbits); \
	register dword_t bits_offset = MODULO_BITS_PER_WORD(symbits)

inline static symbol_t INLINE_ATTRIBUTE symbol_get(const symbol_t* ptr, int idx) {
	GET_PARTS(idx);
	return ((symbol_t)(*((dword_t*)&ptr[word_idx]) >> bits_offset) & SYMBOL_MASK);
};

inline static void INLINE_ATTRIBUTE symbol_put(symbol_t* ptr, int idx, symbol_t sym) {
	GET_PARTS(idx);
	*((dword_t*)&ptr[word_idx]) = ((*((dword_t*)&ptr[word_idx]) & ~((dword_t)SYMBOL_MASK << bits_offset)) |
									((dword_t)(sym & SYMBOL_MASK) << bits_offset));
};

inline static symbol_t INLINE_ATTRIBUTE modnn(symbol_t x)
{
	while (x >= TOTAL_SYMBOL_COUNT) {
		x -= TOTAL_SYMBOL_COUNT;
		x = (x >> BITS_PER_SYMBOL) + (x & SYMBOL_MASK);
	}
	return x;
}

inline static void INLINE_ATTRIBUTE encode_rs(symbol_t data[SYMBOL_TABLE_WORDS]) {
#include <encode_rs.h>
}

typedef struct error_marker {
	uint32_t* pointer;
	uint32_t  corrected_dword;
} error_marker_t;

#define RS_MAX_ERROR_MARKERS (RS_MAX_ERRORS * 2)

inline static int INLINE_ATTRIBUTE decode_rs(symbol_t data[SYMBOL_TABLE_WORDS],
											 int* correction_count,
											 error_marker_t corrections[RS_MAX_ERROR_MARKERS]) {
#include <decode_rs.h>
}


#endif /* #define _REED_SOLOMON_H */
