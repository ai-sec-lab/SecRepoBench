static inline lzw_result lzw__next_code(
		struct lzw_read_ctx *readcontext,
		uint8_t code_size,
		uint32_t *code_out)
{
	uint32_t code = 0;
	uint8_t current_bit = readcontext->sb_bit & 0x7;
	uint8_t byte_advance = (current_bit + code_size) >> 3;
	uint32_t new_code;

	assert(byte_advance <= 2);

	if (readcontext->sb_bit + code_size <= readcontext->sb_bit_count) {
		/* Fast path: code fully inside this sub-block */
		const uint8_t *data = readcontext->sb_data + (readcontext->sb_bit >> 3);
		switch (byte_advance) {
			case 2: code |= data[2] << 16; /* Fall through */
			case 1: code |= data[1] <<  8; /* Fall through */
			case 0: code |= data[0] <<  0;
		}
		readcontext->sb_bit += code_size;
	} else {
		/* Slow path: code spans sub-blocks */
		uint8_t byte = 0;
		uint8_t bits_remaining_0 = (code_size < (8 - current_bit)) ?
				code_size : (8 - current_bit);
		uint8_t bits_remaining_1 = code_size - bits_remaining_0;
		uint8_t bits_used[3] = {
			[0] = bits_remaining_0,
			[1] = bits_remaining_1 < 8 ? bits_remaining_1 : 8,
			[2] = bits_remaining_1 - 8,
		};

		while (true) {
			const uint8_t *data = readcontext->sb_data;
			lzw_result res;

			/* Get any data from end of this sub-block */
			while (byte <= byte_advance &&
					readcontext->sb_bit < readcontext->sb_bit_count) {
				code |= data[readcontext->sb_bit >> 3] << (byte << 3);
				readcontext->sb_bit += bits_used[byte];
				byte++;
			}

			/* Check if we have all we need */
			if (byte > byte_advance) {
				break;
			}

			/* Move to next sub-block */
			res = lzw__block_advance(readcontext);
			if (res != LZW_OK) {
				return res;
			}
		}
	}

	new_code = (code >> current_bit) & ((1 << code_size) - 1);
	if (new_code > ((1 << LZW_CODE_MAX) - 1))
		return LZW_BAD_CODE;
	*code_out = new_code;

	return LZW_OK;
}