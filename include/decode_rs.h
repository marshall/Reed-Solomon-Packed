	int el, i, j, r, k;
	symbol_t q, tmp, num1, num2, den, discr_r;

	symbol_t d0, idx_zero;

	int		 syn_error;
	symbol_t syndromes[PARITY_SYMBOL_COUNT];

	int deg_lambda;
	symbol_t lambda[PARITY_SYMBOL_COUNT + 1], b[PARITY_SYMBOL_COUNT + 1], t[PARITY_SYMBOL_COUNT + 1];

	int		 error_count;
	symbol_t reg[PARITY_SYMBOL_COUNT + 1];

	int deg_omega;
	symbol_t omega[PARITY_SYMBOL_COUNT + 1];

	symbol_t root[PARITY_SYMBOL_COUNT], loc[PARITY_SYMBOL_COUNT];

	int		 corrections;
	word_t 	 offset, word1, word2;

	/* form the syndromes; i.e., evaluate data(x) at roots of g(x) */
	d0 = symbol_get(data, 0);
	for (i = 0; i < PARITY_SYMBOL_COUNT; i++) {
		syndromes[i] = d0;
	}

	for (j = 1; j < TOTAL_SYMBOL_COUNT; j++) {
		for (i = 0; i < PARITY_SYMBOL_COUNT; i++) {
			if (syndromes[i] == 0) {
				syndromes[i] = symbol_get(data, j);
			}
			else {
				syndromes[i] = symbol_get(data, j) ^ symbol_get(alpha_to, modnn(symbol_get(index_of, syndromes[i]) + (FCS + i) * PRIM));
			}
		}
	}

	/* Convert syndromes to index form, checking for nonzero condition */
	syn_error = 0;
	for (i = 0; i < PARITY_SYMBOL_COUNT; i++) {
		syn_error |= syndromes[i];
		syndromes[i] = symbol_get(index_of, syndromes[i]);
	}

	/* if syndrome is zero, data[] is a codeword and there are no
	 * errors to correct. So return data[] unmodified
	 */
	if (syn_error == 0) {
		return 0;
	}

	/* Initialize lambda[] and b[] */
	lambda[0] = 1;
	b[0] = symbol_get(index_of, 1);
	idx_zero = symbol_get(index_of, 0);
	for (i = 1; i <= PARITY_SYMBOL_COUNT; i++) {
		lambda[i] = 0;
		b[i] = idx_zero;
	}

	/*
	 * Begin Berlekamp-Massey algorithm to determine error
	 * locator polynomial
	 */
	r = 0;
	el = 0;
	while (++r <= PARITY_SYMBOL_COUNT) { /* r is the step number */

		/* Compute discrepancy at the r-th step in poly-form */
		discr_r = 0;
		for (i = 0; i < r; i++) {
			if ((lambda[i] != 0) && (syndromes[r - i - 1] != A0)) {
				discr_r ^= symbol_get(alpha_to, modnn(symbol_get(index_of, lambda[i]) + syndromes[r - i - 1]));
			}
		}

		discr_r = symbol_get(index_of, discr_r); /* Index form */
		if (discr_r == A0) {
			/* B(x) <-- x*B(x) */
			for(j = PARITY_SYMBOL_COUNT; j > 0; j--) {
				b[j] = b[j-1];
			}
			b[0] = A0;
		}
		else {
			/* 7 lines below: T(i) <-- lambda(i) - discr_r*i*b(i) */
			t[0] = lambda[0];
			for (i = 0; i < PARITY_SYMBOL_COUNT; i++) {
				if (b[i] != A0) {
					t[i + 1] = lambda[i + 1] ^ symbol_get(alpha_to, modnn(discr_r + b[i]));
				}
				else {
					t[i + 1] = lambda[i + 1];
				}
			}
			if (2 * el <= r - 1) {
				el = r - el;
				/*
				 * 2 lines below: B(x) <-- inv(discr_r) *
				 * lambda(x)
				 */
				for (i = 0; i <= PARITY_SYMBOL_COUNT; i++) {
					b[i] = (lambda[i] == 0) ? A0 : modnn(symbol_get(index_of, lambda[i]) - discr_r + TOTAL_SYMBOL_COUNT);
				}
			}
			else {
				/* B(x) <-- x*B(x) */
				for(j = PARITY_SYMBOL_COUNT; j > 0; j--) {
					b[j] = b[j-1];
				}
				b[0] = A0;
			}

			for(j=0; j<(PARITY_SYMBOL_COUNT + 1); j++) {
				lambda[j] = t[j];
			}
		}
	}

	/* Convert lambda to index form and compute deg(lambda(x)) */
	deg_lambda = 0;
	for (i = 0; i < PARITY_SYMBOL_COUNT + 1; i++) {
		lambda[i] = symbol_get(index_of, lambda[i]);
		if (lambda[i] != A0) {
			deg_lambda = i;
		}
	}

	/* Find roots of the error locator polynomial by Chien search */
	for (i = 1; i <= PARITY_SYMBOL_COUNT; i++) {
		reg[i] = lambda[i];
	}

	error_count = 0;  /* Number of roots of lambda(x) */
	k = IPRIM - 1;
	for (i = 1; i <= TOTAL_SYMBOL_COUNT; i++, k = modnn(k + IPRIM)) {
		q = 1;  /* lambda[0] is always 0 */
		for (j = deg_lambda; j > 0; j--) {
			if (reg[j] != A0) {
				reg[j] = modnn(reg[j] + j);
				q ^= symbol_get(alpha_to, reg[j]);
			}
		}
		if (q == 0) {
			/* store root (index-form) and error location number */
			root[error_count] = i;
			loc[error_count++] = k;

			if(error_count == deg_lambda) {
				break;
			}
		}
	}

	if (deg_lambda != error_count) {
		/* deg(lambda) unequal to number of roots => uncorrectable error detected */
		return -5;
	}

	/*
	 * Compute err evaluator poly omega(x) = syndromes(x)*lambda(x) (modulo
	 * x**PARITY_SYMBOL_COUNT). in index form. Also find deg(omega).
	 */
	deg_omega = deg_lambda - 1;
	for (i = 0; i <= deg_omega; i++) {
		tmp = 0;
		for (j = i; j >= 0; j--) {
			if ((syndromes[i - j] != A0) && (lambda[j] != A0)) {
				tmp ^= symbol_get(alpha_to, modnn(syndromes[i - j] + lambda[j]));
			}
		}
		omega[i] = symbol_get(index_of, tmp);
	}

	/*
	 * Compute error values in poly-form. num1 = omega(inv(X(l))), num2 =
	 * inv(X(l))**(FCR-1) and den = lambda_pr(inv(X(l))) all in poly-form
	 */

	 corrections = 0;
	for (j = error_count - 1; j >= 0; j--) {
		num1 = 0;
		for (i = deg_omega; i >= 0; i--) {
			if (omega[i] != A0) {
				num1  ^= symbol_get(alpha_to, modnn(omega[i] + i * root[j]));
			}
		}
		num2 = symbol_get(alpha_to, modnn(root[j] * (FCS - 1) + TOTAL_SYMBOL_COUNT));
		den = 0;

		/* lambda[i+1] for i even is the formal derivative lambda_pr of lambda[i] */
		for (i = MIN(deg_lambda, PARITY_SYMBOL_COUNT - 1) & ~1; i >= 0; i -= 2) {
			if (lambda[i + 1] != A0) {
				den ^= symbol_get(alpha_to, modnn(lambda[i + 1] + i * root[j]));
			}
		}

		/* Apply error to data */
		if ((num1 != 0)) {
			if (error_count >= RS_MAX_SYMBOL_ERRORS) {
				return -8;
			}

			/* Convert this to word offsets not symbol offsets */
			offset = loc[j] * BITS_PER_SYMBOL / BITS_PER_WORD;
			word1 = data[offset];
			word2 = data[offset+1];
			symbol_put(data, loc[j], symbol_get(data, loc[j]) ^ symbol_get(alpha_to, modnn(symbol_get(index_of, num1) +
																						   symbol_get(index_of, num2) +
																						   TOTAL_SYMBOL_COUNT -
																						   symbol_get(index_of, den))));

			if (word1 != data[offset]) {
				error_offsets[corrections] = offset;
				corrected_values[corrections++] = data[offset];
			}

			if (word2 != data[offset]) {
				error_offsets[corrections] = offset;
				corrected_values[corrections++] = data[offset];
			}
		}
	}

	error_offsets[corrections] = -1;

	return error_count;