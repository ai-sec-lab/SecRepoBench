static inline void crc16_update_block_(FLAC__BitReader *bitreader)
{
	if(bitreader->consumed_words > bitreader->crc16_offset && bitreader->crc16_align)
		crc16_update_word_(bitreader, bitreader->buffer[bitreader->crc16_offset++]);

	/* Prevent OOB read due to wrap-around. */
	// <MASK>
}