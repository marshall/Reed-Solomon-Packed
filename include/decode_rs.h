	int el, i, j, r, k;
	symbol_t q, tmp, num1, num2, den, discr_r;

	symbol_t d0;

	int		 syn_error;
	symbol_t syndromes[PARITY_SYMBOL_COUNT];

	int deg_lambda;
	symbol_t lambda[PARITY_SYMBOL_COUNT + 1], b[PARITY_SYMBOL_COUNT + 1], t[PARITY_SYMBOL_COUNT + 1];

	int		 error_count;
	symbol_t reg[PARITY_SYMBOL_COUNT + 1];

	int deg_omega;
	symbol_t omega[PARITY_SYMBOL_COUNT + 1];

	symbol_t root[PARITY_SYMBOL_COUNT], loc[PARITY_SYMBOL_COUNT];

	uint32_t data_bits_offset, woffs, bit_shift, alignment_needed, symbol_correction;
	uint32_t* tmp32_ptr;
	error_marker_t* em_ptr;

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
	memset(&lambda[1], 0, PARITY_SYMBOL_COUNT * sizeof(lambda[0]));
	lambda[0] = 1;

	for (i = 0; i < PARITY_SYMBOL_COUNT + 1; i++) {
		b[i] = symbol_get(index_of, lambda[i]);
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
		    memmove(&b[1], b, PARITY_SYMBOL_COUNT * sizeof(b[0]));
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
				memmove(&b[1], b, PARITY_SYMBOL_COUNT * sizeof(b[0]));
				b[0] = A0;
			}
      		memcpy(lambda,t,(PARITY_SYMBOL_COUNT + 1) * sizeof(t[0]));
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
	memcpy(&reg[1], &lambda[1], PARITY_SYMBOL_COUNT * sizeof(reg[0]));

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
		return -55;
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

	*correction_count = 0;
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
			if (error_count > RS_MAX_SYMBOL_ERRORS) {
				return -88;
			}

			/* Convert this to uint32_t pointers to uint32_t corrected_values */

			data_bits_offset = (uint32_t)loc[j] * BITS_PER_SYMBOL;
			/* woffs is the index (data[index]) of the first word containing part of this symbol */
			woffs = data_bits_offset / BITS_PER_WORD;
			/* bit_shift is the number of bits that the symbol is offset into the first data word */
			bit_shift = data_bits_offset & 0x0000000F;

			/* symbol_correction XOR'd with the corrupt symbol produces corrected data. */
			symbol_correction = (uint32_t)symbol_get(alpha_to, modnn(symbol_get(index_of, num1) +
																	 symbol_get(index_of, num2) +
																	 TOTAL_SYMBOL_COUNT -
																	 symbol_get(index_of, den)));
			symbol_correction <<= bit_shift;
			tmp32_ptr = (uint32_t*)&data[woffs];

			alignment_needed = ((uint32_t)tmp32_ptr & 2u);

			if (alignment_needed != 0) {
				tmp32_ptr = (uint32_t*)((uint32_t)tmp32_ptr & ~2u);

				em_ptr = get_marker(tmp32_ptr, data, correction_count, corrections);
				em_ptr->corrected_dword ^= (symbol_correction << BITS_PER_WORD);

				em_ptr = get_marker(++tmp32_ptr, data, correction_count, corrections);
				em_ptr->corrected_dword ^= (symbol_correction >> BITS_PER_WORD);
			}
			else {
				em_ptr = get_marker(tmp32_ptr, data, correction_count, corrections);
				em_ptr->corrected_dword ^= symbol_correction;
			}
		}
	}

	return error_count;
