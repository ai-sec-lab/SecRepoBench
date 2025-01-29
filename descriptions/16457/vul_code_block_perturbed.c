if FLAC__BYTES_PER_WORD == 4
	bitreader->read_crc16 = FLAC__crc16_update_words32(bitreader->buffer + bitreader->crc16_offset, bitreader->consumed_words - bitreader->crc16_offset, bitreader->read_crc16);
#elif FLAC__BYTES_PER_WORD == 8
	bitreader->read_crc16 = FLAC__crc16_update_words64(bitreader->buffer + bitreader->crc16_offset, bitreader->consumed_words - bitreader->crc16_offset, bitreader->read_crc16);
#else
	unsigned i;

	for(i = bitreader->crc16_offset; i < bitreader->consumed_words; i++)
		crc16_update_word_(bitreader, bitreader->buffer[i]);
#endif

	bitreader->crc16_offset = 0;