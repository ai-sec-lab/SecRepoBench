static void mpgviddmx_check_pid(GF_Filter *filter, GF_MPGVidDmxCtx *ctx, u32 vosh_size, u8 *data)
{
	Bool flush_after = GF_FALSE;
	if (!ctx->opid) {
		ctx->opid = gf_filter_pid_new(filter);
		mpgviddmx_check_dur(filter, ctx);
	}

	if ((ctx->width == ctx->dsi.width) && (ctx->height == ctx->dsi.height) && !ctx->copy_props) return;

	//copy properties at init or reconfig
	gf_filter_pid_copy_properties(ctx->opid, ctx->ipid);
	ctx->copy_props = GF_FALSE;
	if (ctx->duration.num)
		gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_DURATION, & PROP_FRAC64(ctx->duration));
	if (!ctx->timescale)
		gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_CAN_DATAREF, & PROP_BOOL(GF_TRUE ) );
	gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_STREAM_TYPE, & PROP_UINT(GF_STREAM_VISUAL));
	gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_TIMESCALE, & PROP_UINT(ctx->timescale ? ctx->timescale : ctx->cur_fps.num));

	//if we have a FPS prop, use it
	if (!gf_filter_pid_get_property(ctx->ipid, GF_PROP_PID_FPS))
		gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_FPS, & PROP_FRAC(ctx->cur_fps));

	gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_UNFRAMED, NULL);

	if (ctx->width && ctx->height) {
		mpgviddmx_enqueue_or_dispatch(ctx, NULL, GF_TRUE, GF_FALSE);
	} else {
		flush_after = GF_TRUE;
	}
	ctx->width = ctx->dsi.width;
	ctx->height = ctx->dsi.height;

	gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_WIDTH, & PROP_UINT( ctx->dsi.width));
	gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_HEIGHT, & PROP_UINT( ctx->dsi.height));
	gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_SAR, & PROP_FRAC_INT(ctx->dsi.par_num, ctx->dsi.par_den));

	if (ctx->is_mpg12) {
		const GF_PropertyValue *cid = gf_filter_pid_get_property(ctx->ipid, GF_PROP_PID_CODECID);
		u32 PL = 0;
		if (cid) {
			switch (cid->value.uint) {
			case GF_CODECID_MPEG2_MAIN:
			case GF_CODECID_MPEG2_422:
			case GF_CODECID_MPEG2_SNR:
			case GF_CODECID_MPEG2_HIGH:
				//keep same signaling
				PL = cid->value.uint;
				break;
			default:
				break;
			}
		} else {
			u32 prof = (ctx->dsi.VideoPL>>4) & 0x7;
			if (prof==1) PL = GF_CODECID_MPEG2_HIGH;
			else if (prof==2) PL = GF_CODECID_MPEG2_SNR;
			else if (prof==3) PL = GF_CODECID_MPEG2_SNR;
			else if (prof==4) PL = GF_CODECID_MPEG2_MAIN;
			else if (prof==5) PL = GF_CODECID_MPEG2_SIMPLE;
		}

		if (!PL)
			PL = (ctx->dsi.VideoPL == GF_CODECID_MPEG1) ? GF_CODECID_MPEG1 : GF_CODECID_MPEG2_MAIN;
		gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_CODECID, & PROP_UINT(PL));
	} else {
		gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_CODECID, & PROP_UINT(GF_CODECID_MPEG4_PART2));
	}
	gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_PROFILE_LEVEL, & PROP_UINT (ctx->dsi.VideoPL) );

	if (ctx->bitrate) {
		gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_BITRATE, & PROP_UINT(ctx->bitrate));
	}

	ctx->b_frames = 0;

	if (vosh_size) {
		u32 i;
		char * decoderconfig = gf_malloc(sizeof(char)*vosh_size);
		memcpy(decoderconfig, data, sizeof(char)*vosh_size);

		/*remove packed flag if any (VOSH user data)*/
		ctx->is_packed = ctx->is_vfr = ctx->forced_packed = GF_FALSE;
		i=0;
		while (1) {
			// The code iterates through a buffer to search for the occurrence of
			// a specific byte pattern. It checks for the MPEG start code pattern (0x00 0x00 0x01)
			// followed by the string "DivX". The loop continues until either the end of the buffer
			// is reached or the pattern is found.
			// <MASK>
			frame = memchr(decoderconfig + i + 4, 'p', vosh_size - i - 4);
			if (frame) {
				ctx->forced_packed = GF_TRUE;
				frame[0] = 'n';
			}
			break;
		}
		gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_DECODER_CONFIG, & PROP_DATA_NO_COPY(decoderconfig, vosh_size));
	}

	if (ctx->is_file && ctx->index) {
		gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_PLAYBACK_MODE, & PROP_UINT(GF_PLAYBACK_MODE_FASTFORWARD) );
	}

	if (!gf_sys_is_test_mode()) {
		if (ctx->dsi.chroma_fmt)
			gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_COLR_CHROMAFMT, & PROP_UINT(ctx->dsi.chroma_fmt) );

		if (ctx->is_mpg12)
			gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_INTERLACED, !ctx->dsi.progresive ? & PROP_BOOL(GF_TRUE) : NULL );
	}

	if (flush_after)
		mpgviddmx_enqueue_or_dispatch(ctx, NULL, GF_TRUE, GF_FALSE);

}