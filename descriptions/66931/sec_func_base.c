static void inspect_dump_packet(GF_InspectCtx *ctx, FILE *dump, GF_FilterPacket *pck, u32 pid_idx, u64 pck_num, PidCtx *pctx)
{
	u32 idx=0, size, sap;
	u64 ts;
	u8 dflags = 0;
	GF_FilterClockType ck_type;
	GF_FilterFrameInterface *fifce=NULL;
	Bool start, end;
	u8 *data;

	if (!ctx->dump_log && !dump) return;

	if (!ctx->deep && !ctx->fmt) return;
	if (!ctx->full) {
		inspect_dump_packet_as_info(ctx, dump, pck, pid_idx, pck_num, pctx);
		return;
	}

	data = (u8 *) gf_filter_pck_get_data(pck, &size);
	gf_filter_pck_get_framing(pck, &start, &end);

	ck_type = ctx->pcr ? gf_filter_pck_get_clock_type(pck) : 0;
	if (!size && !ck_type) {
		fifce = gf_filter_pck_get_frame_interface(pck);
	}

	if (ctx->xml) {
		inspect_printf(dump, "<Packet number=\""LLU"\"", pck_num);
		if (ctx->interleave)
			inspect_printf(dump, " PID=\"%d\"", pid_idx);
	} else {
		inspect_printf(dump, "PID %d PCK "LLU" - ", pid_idx, pck_num);
	}
	if (ck_type) {
		ts = gf_filter_pck_get_cts(pck);
		if (ctx->xml) {
			if (ts==GF_FILTER_NO_TS) inspect_printf(dump, " PCR=\"N/A\"");
			else inspect_printf(dump, " PCR=\""LLU"\" ", ts );
			if (ck_type!=GF_FILTER_CLOCK_PCR) inspect_printf(dump, " discontinuity=\"true\"");
			inspect_printf(dump, "/>");
		} else {
			if (ts==GF_FILTER_NO_TS) inspect_printf(dump, " PCR N/A");
			else inspect_printf(dump, " PCR%s "LLU"\n", (ck_type==GF_FILTER_CLOCK_PCR) ? "" : " discontinuity", ts );
		}
		return;
	}
	if (ctx->xml) {
		if (fifce) inspect_printf(dump, " framing=\"interface\"");
		else if (start && end) inspect_printf(dump, " framing=\"complete\"");
		else if (start) inspect_printf(dump, " framing=\"start\"");
		else if (end) inspect_printf(dump, " framing=\"end\"");
		else inspect_printf(dump, " framing=\"continuation\"");
	} else {
		if (fifce) inspect_printf(dump, "interface");
		else if (start && end) inspect_printf(dump, "full frame");
		else if (start) inspect_printf(dump, "frame start");
		else if (end) inspect_printf(dump, "frame end");
		else inspect_printf(dump, "frame continuation");
	}
	ts = gf_filter_pck_get_dts(pck);
	if (ts==GF_FILTER_NO_TS) DUMP_ATT_STR("dts", "N/A")
	else DUMP_ATT_LLU("dts", ts )

	ts = gf_filter_pck_get_cts(pck);
	if (ts==GF_FILTER_NO_TS) DUMP_ATT_STR("cts", "N/A")
	else DUMP_ATT_LLU("cts", ts )

	DUMP_ATT_U("dur", gf_filter_pck_get_duration(pck) )
	sap = gf_filter_pck_get_sap(pck);
	if (sap==GF_FILTER_SAP_4_PROL) {
		DUMP_ATT_STR(sap, "4 (prol)");
	} else {
		DUMP_ATT_U("sap", gf_filter_pck_get_sap(pck) )
	}
	DUMP_ATT_D("ilace", gf_filter_pck_get_interlaced(pck) )
	DUMP_ATT_D("corr", gf_filter_pck_get_corrupted(pck) )
	DUMP_ATT_D("seek", gf_filter_pck_get_seek_flag(pck) )

	ts = gf_filter_pck_get_byte_offset(pck);
	if (ts==GF_FILTER_NO_BO) DUMP_ATT_STR("bo", "N/A")
	else DUMP_ATT_LLU("bo", ts )

	DUMP_ATT_U("roll", gf_filter_pck_get_roll_info(pck) )
	DUMP_ATT_U("crypt", gf_filter_pck_get_crypt_flags(pck) )
	DUMP_ATT_U("vers", gf_filter_pck_get_carousel_version(pck) )

	if (!fifce) {
		DUMP_ATT_U("size", size )
	}
	dflags = gf_filter_pck_get_dependency_flags(pck);
	DUMP_ATT_U("lp", (dflags>>6) & 0x3 )
	DUMP_ATT_U("depo", (dflags>>4) & 0x3 )
	DUMP_ATT_U("depf", (dflags>>2) & 0x3 )
	DUMP_ATT_U("red", (dflags) & 0x3 )

	if (!data) size = 0;
	if (ctx->dump_data) {
		u32 i;
		DUMP_ATT_STR("data", "")
		for (i=0; i<size; i++) {
			inspect_printf(dump, "%02X", (unsigned char) data[i]);
		}
		if (ctx->xml) inspect_printf(dump, "\"");
	} else if (fifce) {
		u32 i;
		char *name = fifce->get_gl_texture ? "Interface_GLTexID" : "Interface_NumPlanes";
		if (ctx->xml) {
			inspect_printf(dump, " %s=\"", name);
		} else {
			inspect_printf(dump, " %s ", name);
		}
		for (i=0; i<4; i++) {
			if (fifce->get_gl_texture) {
				GF_Matrix mx;
				gf_mx_init(mx);
				u32 gl_tex_format, gl_tex_id;
				if (fifce->get_gl_texture(fifce, i, &gl_tex_format, &gl_tex_id, &mx) != GF_OK)
					break;
				if (i) inspect_printf(dump, ",");
				inspect_printf(dump, "%d", gl_tex_id);
			} else {
				u32 stride;
				const u8 *plane;
				if (fifce->get_plane(fifce, i, &plane, &stride) != GF_OK)
					break;
			}
		}
		if (!fifce->get_gl_texture) {
			inspect_printf(dump, "%d", i);
		}

		if (ctx->xml) {
			inspect_printf(dump, "\"");
		}
	} else if (data && (ctx->test!=INSPECT_TEST_NOCRC) ) {
		DUMP_ATT_X("CRC32", gf_crc_32(data, size) )
	}
	if (ctx->xml) {
		if (!ctx->props) goto props_done;

	} else {
		inspect_printf(dump, "\n");
	}

	if (!ctx->props) return;
	while (1) {
		u32 prop_4cc;
		const char *prop_name;
		const GF_PropertyValue * p = gf_filter_pck_enum_properties(pck, &idx, &prop_4cc, &prop_name);
		if (!p) break;
		if (idx==0) inspect_printf(dump, "properties:\n");

		inspect_dump_property(ctx, dump, prop_4cc, prop_name, p, pctx);
	}


props_done:
	if (!ctx->analyze || !size) {
		if (ctx->xml) {
			inspect_printf(dump, "/>\n");
		}
		return;
	}
	inspect_printf(dump, ">\n");

#ifndef GPAC_DISABLE_AV_PARSERS
	if (pctx->hevc_state || pctx->avc_state || pctx->vvc_state) {
		idx=1;

		if (pctx->is_adobe_protected) {
			u8 encrypted_au = data[0];
			if (encrypted_au) {
				inspect_printf(dump, "   <!-- Packet is an Adobe's protected frame and can not be dumped -->\n");
				inspect_printf(dump, "</Packet>\n");
				return;
			}
			else {
				data++;
				size--;
			}
		}
		while (size) {
			if (size < pctx->nalu_size_length) {
				inspect_printf(dump, "   <!-- NALU is corrupted: nalu_size_length is %u but only %d remains -->\n", pctx->nalu_size_length, size);
				break;
			}
			u32 nal_size = inspect_get_nal_size((char*)data, pctx->nalu_size_length);
			data += pctx->nalu_size_length;

			if (nal_size >= GF_UINT_MAX - pctx->nalu_size_length || pctx->nalu_size_length + nal_size > size) {
				inspect_printf(dump, "   <!-- NALU is corrupted: size is %u but only %d remains -->\n", nal_size, size);
				break;
			} else {
				inspect_printf(dump, "   <NALU size=\"%d\" ", nal_size);
				gf_inspect_dump_nalu_internal(dump, data, nal_size, pctx->has_svcc ? 1 : 0, pctx->hevc_state, pctx->avc_state, pctx->vvc_state, pctx->nalu_size_length, ctx->crc, pctx->is_cenc_protected, ctx->analyze, pctx);
			}
			idx++;
			data += nal_size;
			size -= nal_size + pctx->nalu_size_length;
		}
	} else if (pctx->av1_state) {
		gf_bs_reassign_buffer(pctx->bs, data, size);
		while (size) {
			ObuType obu_type = 0;
			u64 obu_size = 0;
			u32 hdr_size = 0;

			obu_size = gf_inspect_dump_obu_internal(dump, pctx->av1_state, (char *) data, size, obu_type, obu_size, hdr_size, ctx->crc, pctx, ctx->analyze);

			if (obu_size > size) {
				inspect_printf(dump, "   <!-- OBU is corrupted: size is %d but only %d remains -->\n", (u32) obu_size, size);
				break;
			}
			data += obu_size;
			size -= (u32)obu_size;
			idx++;
		}
	} else {
		u32 hdr, pos, fsize, i;
		u32 dflag=0;
		switch (pctx->codec_id) {
		case GF_CODECID_MPEG1:
		case GF_CODECID_MPEG2_422:
		case GF_CODECID_MPEG2_SNR:
		case GF_CODECID_MPEG2_HIGH:
		case GF_CODECID_MPEG2_MAIN:
		case GF_CODECID_MPEG2_SIMPLE:
		case GF_CODECID_MPEG2_SPATIAL:
		case GF_CODECID_MPEG4_PART2:
			inspect_dump_mpeg124(pctx, (char *) data, size, dump);
			break;
		case GF_CODECID_MPEG_AUDIO:
		case GF_CODECID_MPEG2_PART3:
		case GF_CODECID_MPEG_AUDIO_L1:
			pos = 0;
			while (size) {
				hdr = gf_mp3_get_next_header_mem(data, size, &pos);
				if (!hdr) {
					inspect_printf(dump, "<!-- Invalid MPEGAudioFrame -->\n");
					break;
				}
				fsize = gf_mp3_frame_size(hdr);
				inspect_printf(dump, "<MPEGAudioFrame size=\"%d\" layer=\"%d\" version=\"%d\" bitrate=\"%d\" channels=\"%d\" samplesPerFrame=\"%d\" samplerate=\"%d\"/>\n", fsize, gf_mp3_layer(hdr), gf_mp3_version(hdr), gf_mp3_bit_rate(hdr), gf_mp3_num_channels(hdr), gf_mp3_window_size(hdr), gf_mp3_sampling_rate(hdr));
				if (size<pos+fsize) break;
				data += pos + fsize;
				size -= pos + fsize;
			}
			break;
		case GF_CODECID_TMCD:
			inspect_dump_tmcd(ctx, pctx, (char *) data, size, dump);
			break;
		case GF_CODECID_SUBS_TEXT:
		case GF_CODECID_META_TEXT:
			dflag=1;
		case GF_CODECID_SUBS_XML:
		case GF_CODECID_META_XML:
			if (dflag)
				inspect_printf(dump, "<![CDATA[");
			for (i=0; i<size; i++) {
				gf_fputc(data[i], dump);
			}
			if (dflag)
				inspect_printf(dump, "]]>\n");
			break;
		case GF_CODECID_APCH:
		case GF_CODECID_APCO:
		case GF_CODECID_APCN:
		case GF_CODECID_APCS:
		case GF_CODECID_AP4X:
		case GF_CODECID_AP4H:
			gf_inspect_dump_prores_internal(dump, (char *) data, size, ctx->crc, pctx);
			break;

		case GF_CODECID_MPHA:
			gf_bs_reassign_buffer(pctx->bs, data, size);
			gf_inspect_dump_mha_frame(dump, pctx->bs, "");
			break;

		case GF_CODECID_MHAS:
			gf_inspect_dump_mhas(dump, (char *) data, size, ctx->crc, pctx);
			break;
		case GF_CODECID_VP8:
			dflag=1;
		case GF_CODECID_VP9:
			inspect_dump_vpx(ctx, dump, (char *) data, size, ctx->crc, pctx, dflag ? 8 : 9);
			break;
		case GF_CODECID_AC3:
			dflag=1;
		case GF_CODECID_EAC3:
			inspect_dump_ac3_eac3(ctx, dump, (char *) data, size, ctx->crc, pctx, dflag ? 0 : 1);
			break;
		case GF_CODECID_TRUEHD:
			gf_bs_reassign_buffer(pctx->bs, data, size);
			gf_inspect_dump_truehd_frame(dump, pctx->bs);
			break;
		case GF_CODECID_OPUS:
			gf_inspect_dump_opus_internal(dump, data, size, 0, ctx->crc, pctx);
			break;
		case GF_CODECID_ALAC:
		{
			gf_bs_reassign_buffer(pctx->bs, data, size);
			u32 val, partial;

#define get_and_print(name, bits) \
			val = gf_bs_read_int(pctx->bs, bits); \
			inspect_printf(dump, " "name"=\"%u\"", val);

			inspect_printf(dump, " <ALACSegment");

			get_and_print("type", 3);
			get_and_print("reserved", 12);
			get_and_print("partial", 1);
			partial=val;
			get_and_print("shift_off", 2);
			get_and_print("escape", 1);
			if (partial) {
				get_and_print("frameLength", 32);
			}
			inspect_printf(dump, "/>\n");

#undef get_and_print

		}
			break;

		}
	}
#endif
	inspect_printf(dump, "</Packet>\n");
}