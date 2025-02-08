static u8 *dnie_uncompress(sc_card_t * card, u8 * source, size_t *len)
{
	u8 *upt = source;
#ifdef ENABLE_ZLIB
	int res = SC_SUCCESS;
	size_t uncompressed = 0L;
	size_t compressed = 0L;

	if (!card || !card->ctx || !source || !len)
		return NULL;
	LOG_FUNC_CALLED(card->ctx);

	/* if data size not enough for compression header assume uncompressed */
	if (*len < 8)
		goto compress_exit;
	/* evaluate compressed an uncompressed sizes (little endian format) */
	uncompressed = lebytes2ulong(source);
	compressed = lebytes2ulong(source + 4);
	/* if compressed size doesn't match data length assume not compressed */
	if (compressed != (*len) - 8)
		goto compress_exit;
	/* if compressed size greater than uncompressed, assume uncompressed data */
	if (uncompressed < compressed)
		goto compress_exit;

	sc_log(card->ctx, "Data seems to be compressed. calling uncompress");
	/* ok: data seems to be compressed */
	upt = calloc(uncompressed, sizeof(u8));
	if (!upt) {
		sc_log(card->ctx, "alloc() for uncompressed buffer failed");
		return NULL;
	}
	// <MASK>
	sc_log_hex(card->ctx, "Uncompressed data", upt, uncompressed);
 compress_exit:

#endif

	sc_log(card->ctx, "uncompress: returning with%s de-compression ",
	       (upt == source) ? "out" : "");
	return upt;
}