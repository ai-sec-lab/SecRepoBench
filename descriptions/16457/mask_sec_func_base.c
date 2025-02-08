static inline void crc16_update_block_(FLAC__BitReader *br)
{
	if(br->consumed_words > br->crc16_offset && br->crc16_align)
		crc16_update_word_(br, br->buffer[br->crc16_offset++]);

	/* Prevent OOB read due to wrap-around. */
	// <MASK>
}