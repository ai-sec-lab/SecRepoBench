u64 obu_size = 0;
			u32 headersize = 0;

			obu_size = gf_inspect_dump_obu_internal(dump, pctx->av1_state, (char *) data, obu_size, obu_type, obu_size, headersize, ctx->crc, pctx, ctx->analyze);

			if (obu_size > size) {
				inspect_printf(dump, "   <!-- OBU is corrupted: size is %d but only %d remains -->\n", (u32) obu_size, size);
				break;
			}