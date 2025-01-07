
		u8 *sync;
		Bool skip_id3v1=GF_FALSE;
		u32 bytes_skipped=0, size, nb_samp, bytes_to_drop=0;

		if (!ctx->tag_size && (remain>3)) {

			/* Did we read an ID3v2 ? */
			if (start[0] == 'I' && start[1] == 'D' && start[2] == '3') {
				if (remain<10)
					return GF_OK;

				ctx->tag_size = ((start[9] & 0x7f) + ((start[8] & 0x7f) << 7) + ((start[7] & 0x7f) << 14) + ((start[6] & 0x7f) << 21));

				bytes_to_drop = 10;
				if (ctx->id3_buffer_alloc < ctx->tag_size+10) {
					ctx->id3_buffer = gf_realloc(ctx->id3_buffer, ctx->tag_size+10);
					ctx->id3_buffer_alloc = ctx->tag_size+10;
				}
				memcpy(ctx->id3_buffer, start, 10);
				ctx->id3_buffer_size = 10;
				goto drop_byte;
			}
		}
		if (ctx->tag_size) {
			if (ctx->tag_size>remain) {
				bytes_to_drop = remain;
				ctx->tag_size-=remain;
			} else {
				bytes_to_drop = ctx->tag_size;
				ctx->tag_size = 0;
			}
			memcpy(ctx->id3_buffer + ctx->id3_buffer_size, start, bytes_to_drop);
			ctx->id3_buffer_size += bytes_to_drop;

			if (!ctx->tag_size && ctx->opid) {
				mp3_dmx_flush_id3(filter, ctx);
			}
			goto drop_byte;

		}

		ctx->hdr = gf_mp3_get_next_header_mem(start, remain, &bytes_skipped);

		//couldn't find sync byte in this packet
		if (!ctx->hdr) {
			break;
		}
		sync = start + bytes_skipped;

		size = gf_mp3_frame_size(ctx->hdr);


		//ready to send packet
		if (size + 1 < remain-bytes_skipped) {
			//make sure we are sync!
			if (sync[size] !=0xFF) {
				if ((sync[size]=='T') && (sync[size+1]=='A') && (sync[size+2]=='G')) {
					skip_id3v1=GF_TRUE;
				} else {
					GF_LOG(ctx->is_sync ? GF_LOG_WARNING : GF_LOG_DEBUG, GF_LOG_MEDIA, ("[MP3Dmx] invalid frame, resyncing\n"));
					ctx->is_sync = GF_FALSE;
					goto drop_byte;
				}
			}
		}
		//otherwise wait for next frame, unless if end of stream
		else if (pck) {
			break;
		}
		//ready to send packet
		mp3_dmx_check_pid(filter, ctx);

		if (!ctx->is_playing) {
			ctx->resume_from = (u32) (sync - ctx->mp3_buffer + 1);
			return GF_OK;
		}
		ctx->is_sync = GF_TRUE;

		nb_samp = gf_mp3_window_size(ctx->hdr);

		if (ctx->in_seek) {
			u64 nb_samples_at_seek = (u64) (ctx->start_range * ctx->sr);
			if (ctx->cts + nb_samp >= nb_samples_at_seek) {
				//u32 samples_to_discard = (ctx->cts + nb_samp ) - nb_samples_at_seek;
				ctx->in_seek = GF_FALSE;
			}
		}

		bytes_to_drop = bytes_skipped + size;
		if (ctx->timescale && !prev_pck_size && (cts != GF_FILTER_NO_TS) ) {
			ctx->cts = cts;
			cts = GF_FILTER_NO_TS;
		}

		if (!ctx->in_seek) {
			if (bytes_skipped + size > remain) {
				GF_LOG(GF_LOG_WARNING, GF_LOG_MEDIA, ("[MP3Dmx] truncated frame of size %u (remains %d)\n", size, remain-bytes_skipped));
				break;
			}
			dst_pck = gf_filter_pck_new_alloc(ctx->opid, size, &output);
			if (!dst_pck) break;
			memcpy(output, sync, size);

			gf_filter_pck_set_cts(dst_pck, ctx->cts);
			if (ctx->timescale && (ctx->timescale!=ctx->sr))
				gf_filter_pck_set_duration(dst_pck, (u32) gf_timestamp_rescale(nb_samp, ctx->sr, ctx->timescale) );
			else
				gf_filter_pck_set_duration(dst_pck, nb_samp);
			gf_filter_pck_set_sap(dst_pck, GF_FILTER_SAP_1);
			gf_filter_pck_set_framing(dst_pck, GF_TRUE, GF_TRUE);

			if (ctx->byte_offset != GF_FILTER_NO_BO) {
				gf_filter_pck_set_byte_offset(dst_pck, ctx->byte_offset + bytes_skipped);
			}

			gf_filter_pck_send(dst_pck);
		}
		mp3_dmx_update_cts(ctx);

		//TODO, parse id3v1 ??
		if (skip_id3v1)
			bytes_to_drop+=128;

		//truncated last frame
		if (bytes_to_drop>remain) {
			if (!is_eos) {
				GF_LOG(GF_LOG_WARNING, GF_LOG_MEDIA, ("[MP3Dmx] truncated frame!\n"));
			}
			bytes_to_drop=remain;
		}

drop_byte:
		if (!bytes_to_drop) {
			bytes_to_drop = 1;
		}
		start += bytes_to_drop;
		remain -= bytes_to_drop;

		if (prev_pck_size) {
			if (prev_pck_size > bytes_to_drop) prev_pck_size -= bytes_to_drop;
			else {
				prev_pck_size=0;
				if (ctx->src_pck) gf_filter_pck_unref(ctx->src_pck);
				ctx->src_pck = pck;
				if (pck)
					gf_filter_pck_ref_props(&ctx->src_pck);
			}
		}
		if (ctx->byte_offset != GF_FILTER_NO_BO)
			ctx->byte_offset += bytes_to_drop;
	