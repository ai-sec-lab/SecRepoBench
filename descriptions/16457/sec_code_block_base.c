if (br->consumed_words > br->crc16_offset) {
#if FLAC__BYTES_PER_WORD == 4
		br->read_crc16 = FLAC__crc16_update_words32(br->buffer + br->crc16_offset, br->consumed_words - br->crc16_offset, br->read_crc16);
#elif FLAC__BYTES_PER_WORD == 8
		br->read_crc16 = FLAC__crc16_update_words64(br->buffer + br->crc16_offset, br->consumed_words - br->crc16_offset, br->read_crc16);
#else
		unsigned i;

		for (i = br->crc16_offset; i < br->consumed_words; i++)
			crc16_update_word_(br, br->buffer[i]);
#endif
	}

	br->crc16_offset = 0;