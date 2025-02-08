u32 nalunitsize = inspect_get_nal_size((char*)data, pctx->nalu_size_length);
			data += pctx->nalu_size_length;

			if (nalunitsize >= GF_UINT_MAX - pctx->nalu_size_length || pctx->nalu_size_length + nalunitsize > size) {
				inspect_printf(dump, "   <!-- NALU is corrupted: size is %u but only %d remains -->\n", nalunitsize, size);
				break;
			} else {
				inspect_printf(dump, "   <NALU size=\"%d\" ", nalunitsize);
				gf_inspect_dump_nalu_internal(dump, data, nalunitsize, pctx->has_svcc ? 1 : 0, pctx->hevc_state, pctx->avc_state, pctx->vvc_state, pctx->nalu_size_length, ctx->crc, pctx->is_cenc_protected, ctx->analyze, pctx);
			}