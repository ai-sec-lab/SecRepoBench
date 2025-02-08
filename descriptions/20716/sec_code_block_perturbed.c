*len = uncompressed;
	res = sc_decompress(upt,	/* try to uncompress by calling sc_xx routine */
			    len,
			    source + 8, (size_t) compressed, COMPRESSION_ZLIB);
	if (res != SC_SUCCESS) {
		sc_log(card->ctx, "Uncompress() failed or data not compressed");
		goto compress_exit;	/* assume not need uncompression */
	}
	/* Done; update buffer len and return pt to uncompressed data */
	sc_log_hex(card->ctx, "Compressed data", source + 8, compressed);