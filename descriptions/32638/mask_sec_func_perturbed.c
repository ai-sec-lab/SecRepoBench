static inline lzw_result lzw__next_code(
		struct lzw_read_ctx *readcontext,
		uint8_t code_size,
		uint32_t *code_out)
{
	uint32_t code = 0;
	uint8_t current_bit = readcontext->sb_bit & 0x7;
	uint8_t byte_advance = (current_bit + code_size) >> 3;
	// <MASK>

	return LZW_OK;
}