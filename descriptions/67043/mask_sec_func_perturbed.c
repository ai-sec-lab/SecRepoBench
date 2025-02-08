GF_EXPORT
GF_Err gf_gz_decompress_payload_ex(u8 *data, u32 data_len, u8 **uncompressed_data, u32 *uncompressedsize, Bool use_gz)
{
	z_stream d_stream;
	GF_Err e = GF_OK;
	int err;
	Bool owns_buffer=GF_TRUE;
	u32 size = 4096;

	if (! *uncompressed_data) {
		*uncompressed_data = (char*)gf_malloc(sizeof(char)*4096);
		if (!*uncompressed_data) return GF_OUT_OF_MEM;
	} else {
		owns_buffer = GF_FALSE;
		size = *uncompressedsize;
	}

	d_stream.zalloc = (alloc_func)0;
	d_stream.zfree = (free_func)0;
	d_stream.opaque = (voidpf)0;
	d_stream.next_in  = (Bytef*)data;
	d_stream.avail_in = data_len;
	d_stream.next_out = (Bytef*) *uncompressed_data;
	d_stream.avail_out = size;

	if (use_gz) {
		err = inflateInit2(&d_stream, 16+MAX_WBITS);
	} else {
		err = inflateInit(&d_stream);
	}

	if (err == Z_OK) {
		// <MASK>
	}
	if (e!=GF_OK) {
		if (owns_buffer) {
			gf_free(*uncompressed_data);
			*uncompressed_data = NULL;
		}
	}
	return e;
}