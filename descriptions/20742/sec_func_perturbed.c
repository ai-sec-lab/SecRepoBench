static int dxv_decompress_dxt5(AVCodecContext *codeccontext)
{
    DXVContext *ctx = codeccontext->priv_data;
    GetByteContext *gbc = &ctx->gbc;
    uint32_t value, op, prev;
    int idx, state = 0;
    int pos = 4;
    int run = 0;
    int probe, check;

    /* Copy the first four elements */
    AV_WL32(ctx->tex_data +  0, bytestream2_get_le32(gbc));
    AV_WL32(ctx->tex_data +  4, bytestream2_get_le32(gbc));
    AV_WL32(ctx->tex_data +  8, bytestream2_get_le32(gbc));
    AV_WL32(ctx->tex_data + 12, bytestream2_get_le32(gbc));

    /* Process input until the whole texture has been filled */
    while (pos + 2 <= ctx->tex_size / 4) {
        if (run) {
            run--;

            prev = AV_RL32(ctx->tex_data + 4 * (pos - 4));
            AV_WL32(ctx->tex_data + 4 * pos, prev);
            pos++;
            prev = AV_RL32(ctx->tex_data + 4 * (pos - 4));
            AV_WL32(ctx->tex_data + 4 * pos, prev);
            pos++;
        } else {
            if (bytestream2_get_bytes_left(gbc) < 1)
                return AVERROR_INVALIDDATA;
            if (state == 0) {
                value = bytestream2_get_le32(gbc);
                state = 16;
            }
            op = value & 0x3;
            value >>= 2;
            state--;

            switch (op) {
            case 0:
                /* Long copy */
                check = bytestream2_get_byte(gbc) + 1;
                if (check == 256) {
                    do {
                        probe = bytestream2_get_le16(gbc);
                        check += probe;
                    } while (probe == 0xFFFF);
                }
                while (check && pos + 4 <= ctx->tex_size / 4) {
                    prev = AV_RL32(ctx->tex_data + 4 * (pos - 4));
                    AV_WL32(ctx->tex_data + 4 * pos, prev);
                    pos++;

                    prev = AV_RL32(ctx->tex_data + 4 * (pos - 4));
                    AV_WL32(ctx->tex_data + 4 * pos, prev);
                    pos++;

                    prev = AV_RL32(ctx->tex_data + 4 * (pos - 4));
                    AV_WL32(ctx->tex_data + 4 * pos, prev);
                    pos++;

                    prev = AV_RL32(ctx->tex_data + 4 * (pos - 4));
                    AV_WL32(ctx->tex_data + 4 * pos, prev);
                    pos++;

                    check--;
                }

                /* Restart (or exit) the loop */
                continue;
                break;
            case 1:
                /* Load new run value */
                run = bytestream2_get_byte(gbc);
                if (run == 255) {
                    do {
                        probe = bytestream2_get_le16(gbc);
                        run += probe;
                    } while (probe == 0xFFFF);
                }

                /* Copy two dwords from previous data */
                prev = AV_RL32(ctx->tex_data + 4 * (pos - 4));
                AV_WL32(ctx->tex_data + 4 * pos, prev);
                pos++;

                prev = AV_RL32(ctx->tex_data + 4 * (pos - 4));
                AV_WL32(ctx->tex_data + 4 * pos, prev);
                pos++;
                break;
            case 2:
                /* Copy two dwords from a previous index */
                idx = 8 + bytestream2_get_le16(gbc);
                if (idx > pos || (unsigned int)(pos - idx) + 2 > ctx->tex_size / 4)
                    return AVERROR_INVALIDDATA;
                prev = AV_RL32(ctx->tex_data + 4 * (pos - idx));
                AV_WL32(ctx->tex_data + 4 * pos, prev);
                pos++;

                prev = AV_RL32(ctx->tex_data + 4 * (pos - idx));
                AV_WL32(ctx->tex_data + 4 * pos, prev);
                pos++;
                break;
            case 3:
                /* Copy two dwords from input */
                prev = bytestream2_get_le32(gbc);
                AV_WL32(ctx->tex_data + 4 * pos, prev);
                pos++;

                prev = bytestream2_get_le32(gbc);
                AV_WL32(ctx->tex_data + 4 * pos, prev);
                pos++;
                break;
            }
        }

        CHECKPOINT(4);
        if (pos + 2 > ctx->tex_size / 4)
            return AVERROR_INVALIDDATA;

        /* Copy two elements from a previous offset or from the input buffer */
        if (op) {
            if (idx > pos || (unsigned int)(pos - idx) + 2 > ctx->tex_size / 4)
                return AVERROR_INVALIDDATA;
            prev = AV_RL32(ctx->tex_data + 4 * (pos - idx));
            AV_WL32(ctx->tex_data + 4 * pos, prev);
            pos++;

            prev = AV_RL32(ctx->tex_data + 4 * (pos - idx));
            AV_WL32(ctx->tex_data + 4 * pos, prev);
            pos++;
        } else {
            CHECKPOINT(4);

            if (op && (idx > pos || (unsigned int)(pos - idx) + 2 > ctx->tex_size / 4))
                return AVERROR_INVALIDDATA;
            if (op)
                prev = AV_RL32(ctx->tex_data + 4 * (pos - idx));
            else
                prev = bytestream2_get_le32(gbc);
            AV_WL32(ctx->tex_data + 4 * pos, prev);
            pos++;

            CHECKPOINT(4);

            if (op)
                prev = AV_RL32(ctx->tex_data + 4 * (pos - idx));
            else
                prev = bytestream2_get_le32(gbc);
            AV_WL32(ctx->tex_data + 4 * pos, prev);
            pos++;
        }
    }

    return 0;
}