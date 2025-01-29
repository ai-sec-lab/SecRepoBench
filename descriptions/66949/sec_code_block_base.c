u32 nal_size = inspect_get_nal_size((char*)data, pctx->nalu_size_length);
			data += pctx->nalu_size_length;

			if (nal_size >= GF_UINT_MAX - pctx->nalu_size_length || pctx->nalu_size_length + nal_size > size) {
				inspect_printf(dump, "   <!-- NALU is corrupted: size is %u but only %d remains -->\n", nal_size, size);
				break;
			} else {
				inspect_printf(dump, "   <NALU size=\"%d\" ", nal_size);
				gf_inspect_dump_nalu_internal(dump, data, nal_size, pctx->has_svcc ? 1 : 0, pctx->hevc_state, pctx->avc_state, pctx->vvc_state, pctx->nalu_size_length, ctx->crc, pctx->is_cenc_protected, ctx->analyze, pctx);
			}