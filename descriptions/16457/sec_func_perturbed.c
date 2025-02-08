static inline void crc16_update_block_(FLAC__BitReader *bitreader)
{
	if(bitreader->consumed_words > bitreader->crc16_offset && bitreader->crc16_align)
		crc16_update_word_(bitreader, bitreader->buffer[bitreader->crc16_offset++]);

	/* Prevent OOB read due to wrap-around. */
	if (bitreader->consumed_words > bitreader->crc16_offset) {
#if FLAC__BYTES_PER_WORD == 4
		bitreader->read_crc16 = FLAC__crc16_update_words32(bitreader->buffer + bitreader->crc16_offset, bitreader->consumed_words - bitreader->crc16_offset, bitreader->read_crc16);
#elif FLAC__BYTES_PER_WORD == 8
		bitreader->read_crc16 = FLAC__crc16_update_words64(bitreader->buffer + bitreader->crc16_offset, bitreader->consumed_words - bitreader->crc16_offset, bitreader->read_crc16);
#else
		unsigned i;

		for (i = bitreader->crc16_offset; i < bitreader->consumed_words; i++)
			crc16_update_word_(bitreader, bitreader->buffer[i]);
#endif
	}

	bitreader->crc16_offset = 0;
}