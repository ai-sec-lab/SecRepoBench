while (d_stream.total_in < data_len) {
			err = inflate(&d_stream, Z_NO_FLUSH);
			if (err < Z_OK || err == Z_NEED_DICT) {
				e = GF_NON_COMPLIANT_BITSTREAM;
				break;
			}
			if (err==Z_STREAM_END) break;

			size *= 2;
			*uncompressed_data = (char*)gf_realloc(*uncompressed_data, sizeof(char)*(size+1));
			if (!*uncompressed_data) return GF_OUT_OF_MEM;
			d_stream.avail_out = (u32) (size - d_stream.total_out);
			d_stream.next_out = (Bytef*) ( *uncompressed_data + d_stream.total_out);
		}
		*out_size = (u32) d_stream.total_out;
		inflateEnd(&d_stream);
		//force last byte to 0
		(*uncompressed_data)[*out_size] = 0;