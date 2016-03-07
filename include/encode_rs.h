	int i, j;
	symbol_t feedback;

	for(j=0; j < PARITY_SYMBOL_COUNT; j++) {
		symbol_put(data, PARITY_SYMBOLS(j), 0);
	}

	for (i = 0; i < DATA_SYMBOL_COUNT; i++) {
		feedback = symbol_get(index_of, symbol_get(data, i) ^ symbol_get(data, PARITY_SYMBOLS(0)));
		if (feedback != A0) { /* feedback term is non-zero */
			for (j = 1; j < PARITY_SYMBOL_COUNT; j++) {
				symbol_put(data, PARITY_SYMBOLS(j), symbol_get(data, PARITY_SYMBOLS(j)) ^ symbol_get(alpha_to, modnn(feedback + genpoly[PARITY_SYMBOL_COUNT - j])));
			}
		}
		/* Shift */
		for(j=0; j < PARITY_SYMBOL_COUNT - 1; j++) {
			symbol_put(data, PARITY_SYMBOLS(j), symbol_get(data, PARITY_SYMBOLS(j + 1)));
		}
		if (feedback != A0) {
			symbol_put(data, PARITY_SYMBOLS(PARITY_SYMBOL_COUNT - 1), symbol_get(alpha_to, modnn(feedback + genpoly[0])));
		}
		else {
			symbol_put(data, PARITY_SYMBOLS(PARITY_SYMBOL_COUNT - 1), 0);
		}
	}
