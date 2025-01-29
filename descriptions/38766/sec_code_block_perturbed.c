/* The "lzma alone" file format and the stream format inside ZIPx are
	 * almost the same. Here's an example of a structure of "lzma alone"
	 * format:
	 *
	 * $ cat /bin/ls | lzma | xxd | head -n 1
	 * 00000000: 5d00 0080 00ff ffff ffff ffff ff00 2814
	 *
	 *    5 bytes        8 bytes        n bytes
	 * <lzma_params><uncompressed_size><data...>
	 *
	 * lzma_params is a 5-byte blob that has to be decoded to extract
	 * parameters of this LZMA stream. The uncompressed_size field is an
	 * uint64_t value that contains information about the size of the
	 * uncompressed file, or UINT64_MAX if this value is unknown.
	 * The <data...> part is the actual lzma-compressed data stream.
	 *
	 * Now here's the structure of the stream inside the ZIPX file:
	 *
	 * $ cat stream_inside_zipx | xxd | head -n 1
	 * 00000000: 0914 0500 5d00 8000 0000 2814 .... ....
	 *
	 *  2byte   2byte    5 bytes     n bytes
	 * <magic1><magic2><lzma_params><data...>
	 *
	 * This means that the ZIPX file contains an additional magic1 and
	 * magic2 headers, the lzma_params field contains the same parameter
	 * set as in the "lzma alone" format, and the <data...> field is the
	 * same as in the "lzma alone" format as well. Note that also the zipx
	 * format is missing the uncompressed_size field.
	 *
	 * So, in order to use the "lzma alone" decoder for the zipx lzma
	 * stream, we simply need to shuffle around some fields, prepare a new
	 * lzma alone header, feed it into lzma alone decoder so it will
	 * initialize itself properly, and then we can start feeding normal
	 * zipx lzma stream into the decoder.
	 */

	/* Read magic1,magic2,lzma_params from the ZIPX stream. */
	if(zip->entry_bytes_remaining < 9 || (p = __archive_read_ahead(a, 9, NULL)) == NULL) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
		    "Truncated lzma data");
		return (ARCHIVE_FATAL);
	}