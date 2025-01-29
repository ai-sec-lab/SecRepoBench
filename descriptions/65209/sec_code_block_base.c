u64 obu_size = 0;
			u32 hdr_size = 0;

			obu_size = gf_inspect_dump_obu_internal(dump, pctx->av1_state, (char *) data, size, obu_type, obu_size, hdr_size, ctx->crc, pctx, ctx->analyze);

			if (obu_size > size) {
				inspect_printf(dump, "   <!-- OBU is corrupted: size is %d but only %d remains -->\n", (u32) obu_size, size);
				break;
			}