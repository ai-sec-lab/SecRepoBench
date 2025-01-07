

			memcpy(ctx->hdr_store + ctx->bytes_in_header, start, MIN_HDR_STORE - ctx->bytes_in_header);
			current = mpgviddmx_next_start_code(ctx->hdr_store, MIN_HDR_STORE);

			//no start code in stored buffer
			if ((current<0) || (current >= (s32) ctx->bytes_in_header) )  {
				if (ctx->opid) {
					dst_pck = gf_filter_pck_new_alloc(ctx->opid, ctx->bytes_in_header, &pck_data);
					if (!dst_pck) return GF_OUT_OF_MEM;

					if (ctx->src_pck) gf_filter_pck_merge_properties(ctx->src_pck, dst_pck);
					gf_filter_pck_set_cts(dst_pck, GF_FILTER_NO_TS);
					gf_filter_pck_set_dts(dst_pck, GF_FILTER_NO_TS);
					gf_filter_pck_set_sap(dst_pck, GF_FILTER_SAP_NONE);
					memcpy(pck_data, ctx->hdr_store, ctx->bytes_in_header);
					gf_filter_pck_set_framing(dst_pck, GF_FALSE, GF_FALSE);

					if (byte_offset != GF_FILTER_NO_BO) {
						gf_filter_pck_set_byte_offset(dst_pck, byte_offset - ctx->bytes_in_header);
					}

					mpgviddmx_enqueue_or_dispatch(ctx, dst_pck, GF_FALSE, GF_FALSE);
				}

				if (current<0) current = -1;
				else current -= ctx->bytes_in_header;
				ctx->bytes_in_header = 0;
			} else {
				//we have a valid start code, check which byte in our store or in the packet payload is the start code type
				//and remember its location to reinit the parser from there
				hdr_offset = 4 - ctx->bytes_in_header + current;
				//bytes still to dispatch
				bytes_from_store = ctx->bytes_in_header;
				ctx->bytes_in_header = 0;
				if (!hdr_offset) {
					forced_sc_type = ctx->hdr_store[current+3];
				} else {
					forced_sc_type = start[hdr_offset-1];
				}
				sc_type_forced = GF_TRUE;
			}
		