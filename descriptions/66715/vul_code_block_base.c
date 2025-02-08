u32 size, w=0, h=0, pf=0;
	u8 *pix;
	u32 i, j, irow, in_stride, out_stride;
	GF_BitStream *bs;
	BITMAPFILEHEADER fh;
	BITMAPINFOHEADER fi;

	pck = gf_filter_pid_get_packet(ctx->ipid);
	if (!pck) {
		if (gf_filter_pid_is_eos(ctx->ipid)) {
			if (ctx->opid)
				gf_filter_pid_set_eos(ctx->opid);
			ctx->is_playing = GF_FALSE;
			return GF_EOS;
		}
		return GF_OK;
	}
	data = (char *) gf_filter_pck_get_data(pck, &size);