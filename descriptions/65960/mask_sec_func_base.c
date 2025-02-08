void id3dmx_flush(GF_Filter *filter, u8 *id3_buf, u32 id3_buf_size, GF_FilterPid *audio_pid, GF_FilterPid **video_pid_p)
{
	GF_BitStream *bs = gf_bs_new(id3_buf, id3_buf_size, GF_BITSTREAM_READ);
	char *sep_desc;
	char *_buf=NULL;
	u32 buf_alloc=0;
	gf_bs_skip_bytes(bs, 3);
	/*u8 major = */gf_bs_read_u8(bs);
	/*u8 minor = */gf_bs_read_u8(bs);
	/*u8 unsync = */gf_bs_read_int(bs, 1);
	u8 ext_hdr = gf_bs_read_int(bs, 1);
	gf_bs_read_int(bs, 6);
	u32 size = gf_id3_read_size(bs);


	if (ext_hdr) {

	}

	while (size && (gf_bs_available(bs)>=10) ) {
		char *buf;
		char szTag[1024];
		char *sep;
		s32 tag_idx;
		u32 pic_size;
		//u32 pic_type;
		u32 ftag = gf_bs_read_u32(bs);
		u32 fsize = gf_id3_read_size(bs);
		/*u16 fflags = */gf_bs_read_u16(bs);

		size -= 10;
		if (!fsize)
			break;

		if (size<fsize) {
			GF_LOG(GF_LOG_ERROR, GF_LOG_MEDIA, ("[MP3Dmx] Broken ID3 frame tag %s, size %d but remaining bytes %d\n", gf_4cc_to_str(ftag), fsize, size));
			break;
		}

		if (buf_alloc <= fsize+3) {
			_buf = gf_realloc(_buf, fsize+4);
			buf_alloc = fsize+4;
		}
		//read into _buf+1 so that buf+1 is always %2 mem aligned as it can be loaded as unsigned short
		// having 3 nulls allows for correct 2-char-wide null terminating string not matter the alignment
		gf_bs_read_data(bs, _buf+1, fsize);
		_buf[fsize+1]=0;
		_buf[fsize+2]=0;
		_buf[fsize+3]=0;
		buf = _buf+1;

		tag_idx = gf_itags_find_by_id3tag(ftag);

		if (ftag==GF_ID3V2_FRAME_TXXX) {
			sep = memchr(buf, 0, fsize);
			if (sep) {
				if (!stricmp(buf+1, "comment")) {
					id3dmx_set_string(audio_pid, "comment", sep+1, GF_FALSE);
				} else {
					strcpy(szTag, "tag_");
					strncat(szTag, buf+1, 1019);
					id3dmx_set_string(audio_pid, szTag, sep+1, GF_TRUE);
				}
			}
		} else if (ftag == GF_ID3V2_FRAME_APIC) {
			//first char is text encoding
			//then mime
			sep = memchr(buf+1, 0, fsize-1);
			/*pic_type = sep[1];*/
			sep_desc = sep ? memchr(sep+2, 0, fsize-1) : NULL;

			if (sep_desc) {
				// <MASK>
			}
		} else if (tag_idx>=0) {
			const char *tag_name = gf_itags_get_name((u32) tag_idx);
			id3dmx_set_string(audio_pid, (char *) tag_name, buf+1, GF_FALSE);
		} else {
			sprintf(szTag, "tag_%s", gf_4cc_to_str(ftag));
			if ((ftag>>24) == 'T') {
				id3dmx_set_string(audio_pid, szTag, buf+1, GF_TRUE);
			} else {
				gf_filter_pid_set_property_dyn(audio_pid, szTag, &PROP_DATA(buf, fsize) );
			}
		}
		size -= fsize;
	}
	gf_bs_del(bs);
	if (_buf) gf_free(_buf);
}