res = sc_decompress(upt,	/* try to uncompress by calling sc_xx routine */
			    (size_t *) & uncompressed,
			    source + 8, (size_t) compressed, COMPRESSION_ZLIB);
	/* TODO: check that returned uncompressed size matches expected */
	if (res != SC_SUCCESS) {
		sc_log(card->ctx, "Uncompress() failed or data not compressed");
		goto compress_exit;	/* assume not need uncompression */
	}
	/* Done; update buffer len and return pt to uncompressed data */
	*len = uncompressed;
	sc_log_hex(card->ctx, "Compressed data", source + 8, compressed);