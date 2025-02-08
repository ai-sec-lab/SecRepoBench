static int decode_frame(AVCodecContext *avctx, AVFrame *p,
                        int *got_frame, AVPacket *avpkt)
{
    GetByteContext bytectx;
    int colors;
    int w, h, ret;
    int ver;

    bytestream2_init(&bytectx, avpkt->data, avpkt->size);
    if (   bytestream2_get_bytes_left(&bytectx) >= 552
           &&  check_header(bytectx.buffer + 512, bytestream2_get_bytes_left(&bytectx) - 512)
       )
        bytestream2_skip(&bytectx, 512);

    ver = check_header(bytectx.buffer, bytestream2_get_bytes_left(&bytectx));

    /* smallest PICT header */
    if (bytestream2_get_bytes_left(&bytectx) < 40) {
        av_log(avctx, AV_LOG_ERROR, "Frame is too small %d\n",
               bytestream2_get_bytes_left(&bytectx));
        return AVERROR_INVALIDDATA;
    }

    bytestream2_skip(&bytectx, 6);
    h = bytestream2_get_be16(&bytectx);
    w = bytestream2_get_be16(&bytectx);

    ret = ff_set_dimensions(avctx, w, h);
    if (ret < 0)
        return ret;

    /* version 1 is identified by 0x1101
     * it uses byte-aligned opcodes rather than word-aligned */
    if (ver == 1) {
        avpriv_request_sample(avctx, "QuickDraw version 1");
        return AVERROR_PATCHWELCOME;
    } else if (ver != 2) {
        avpriv_request_sample(avctx, "QuickDraw version unknown (%X)", bytestream2_get_be32(&bytectx));
        return AVERROR_PATCHWELCOME;
    }

    bytestream2_skip(&bytectx, 4+26);

    while (bytestream2_get_bytes_left(&bytectx) >= 4) {
        int bppcnt, bpp;
        int rowbytes, pack_type;
        int flags;
        int opcode = bytestream2_get_be16(&bytectx);

        switch(opcode) {
        case CLIP:
            bytestream2_skip(&bytectx, 10);
            break;
        case PACKBITSRECT:
        case PACKBITSRGN:
            av_log(avctx, AV_LOG_DEBUG, "Parsing Packbit opcode\n");

            flags = bytestream2_get_be16(&bytectx) & 0xC000;
            bytestream2_skip(&bytectx, 28);
            bppcnt = bytestream2_get_be16(&bytectx); /* cmpCount */
            bpp    = bytestream2_get_be16(&bytectx); /* cmpSize */

            av_log(avctx, AV_LOG_DEBUG, "bppcount %d bpp %d\n", bppcnt, bpp);
            if (bppcnt == 1 && bpp == 8) {
                avctx->pix_fmt = AV_PIX_FMT_PAL8;
            } else if (bppcnt == 1 && (bpp == 4 || bpp == 2)) {
                avctx->pix_fmt = AV_PIX_FMT_PAL8;
            } else if (bppcnt == 3 && bpp == 5) {
                avctx->pix_fmt = AV_PIX_FMT_RGB555;
            } else {
                av_log(avctx, AV_LOG_ERROR,
                       "Invalid pixel format (bppcnt %d bpp %d) in Packbit\n",
                       bppcnt, bpp);
                return AVERROR_INVALIDDATA;
            }

            /* jump to palette */
            bytestream2_skip(&bytectx, 18);
            colors = bytestream2_get_be16(&bytectx);

            if (colors < 0 || colors > 255) {
                av_log(avctx, AV_LOG_ERROR,
                       "Error color count - %i(0x%X)\n", colors, colors);
                return AVERROR_INVALIDDATA;
            }
            if (bytestream2_get_bytes_left(&bytectx) < (colors + 1) * 8) {
                av_log(avctx, AV_LOG_ERROR, "Palette is too small %d\n",
                       bytestream2_get_bytes_left(&bytectx));
                return AVERROR_INVALIDDATA;
            }
            if ((ret = ff_get_buffer(avctx, p, 0)) < 0)
                return ret;

            ret = parse_palette(avctx, &bytectx, (uint32_t *)p->data[1], colors, flags & 0x8000);
            if (ret < 0)
                return ret;
            p->palette_has_changed = 1;

            /* jump to image data */
            bytestream2_skip(&bytectx, 18);

            if (opcode == PACKBITSRGN) {
                bytestream2_skip(&bytectx, 2 + 8); /* size + rect */
                avpriv_report_missing_feature(avctx, "Packbit mask region");
            }

            if (avctx->pix_fmt == AV_PIX_FMT_RGB555)
                ret = decode_rle16(avctx, p, &bytectx);
            else if (bpp == 2)
                ret = decode_rle_bpp2(avctx, p, &bytectx);
            else if (bpp == 4)
                ret = decode_rle_bpp4(avctx, p, &bytectx);
            else
                ret = decode_rle(avctx, p, &bytectx, bppcnt);
            if (ret < 0)
                return ret;
            *got_frame = 1;
            break;
        case DIRECTBITSRECT:
        case DIRECTBITSRGN:
            av_log(avctx, AV_LOG_DEBUG, "Parsing Directbit opcode\n");

            bytestream2_skip(&bytectx, 4);
            rowbytes = bytestream2_get_be16(&bytectx) & 0x3FFF;
            if (rowbytes <= 250) {
                avpriv_report_missing_feature(avctx, "Short rowbytes");
                return AVERROR_PATCHWELCOME;
            }

            bytestream2_skip(&bytectx, 4);
            h = bytestream2_get_be16(&bytectx);
            w = bytestream2_get_be16(&bytectx);
            bytestream2_skip(&bytectx, 2);

            ret = ff_set_dimensions(avctx, w, h);
            if (ret < 0)
                return ret;

            pack_type = bytestream2_get_be16(&bytectx);

            bytestream2_skip(&bytectx, 16);
            bppcnt = bytestream2_get_be16(&bytectx); /* cmpCount */
            bpp    = bytestream2_get_be16(&bytectx); /* cmpSize */

            av_log(avctx, AV_LOG_DEBUG, "bppcount %d bpp %d\n", bppcnt, bpp);
            if (bppcnt == 3 && bpp == 8) {
                avctx->pix_fmt = AV_PIX_FMT_RGB24;
            } else if (bppcnt == 3 && bpp == 5 || bppcnt == 2 && bpp == 8) {
                avctx->pix_fmt = AV_PIX_FMT_RGB555;
            } else if (bppcnt == 4 && bpp == 8) {
                avctx->pix_fmt = AV_PIX_FMT_ARGB;
            } else {
                av_log(avctx, AV_LOG_ERROR,
                       "Invalid pixel format (bppcnt %d bpp %d) in Directbit\n",
                       bppcnt, bpp);
                return AVERROR_INVALIDDATA;
            }

            /* set packing when default is selected */
            if (pack_type == 0)
                pack_type = bppcnt;

            if (pack_type != 3 && pack_type != 4) {
                avpriv_request_sample(avctx, "Pack type %d", pack_type);
                return AVERROR_PATCHWELCOME;
            }
            if (bytestream2_get_bytes_left(&bytectx) < 30)
                return AVERROR_INVALIDDATA;
            if ((ret = ff_get_buffer(avctx, p, 0)) < 0)
                return ret;

            /* jump to data */
            bytestream2_skip(&bytectx, 30);

            if (opcode == DIRECTBITSRGN) {
                bytestream2_skip(&bytectx, 2 + 8); /* size + rect */
                avpriv_report_missing_feature(avctx, "DirectBit mask region");
            }

            if (avctx->pix_fmt == AV_PIX_FMT_RGB555)
                ret = decode_rle16(avctx, p, &bytectx);
            else
                ret = decode_rle(avctx, p, &bytectx, bppcnt);
            if (ret < 0)
                return ret;
            *got_frame = 1;
            break;
        case LONGCOMMENT:
            bytestream2_get_be16(&bytectx);
            bytestream2_skip(&bytectx, bytestream2_get_be16(&bytectx));
            break;
        default:
            av_log(avctx, AV_LOG_TRACE, "Unknown 0x%04X opcode\n", opcode);
            break;
        }
        /* exit the loop when a known pixel block has been found */
        if (*got_frame) {
            int eop, trail;

            /* re-align to a word */
            bytestream2_skip(&bytectx, bytestream2_get_bytes_left(&bytectx) % 2);

            eop = bytestream2_get_be16(&bytectx);
            trail = bytestream2_get_bytes_left(&bytectx);
            if (eop != EOP)
                av_log(avctx, AV_LOG_WARNING,
                       "Missing end of picture opcode (found 0x%04X)\n", eop);
            if (trail)
                av_log(avctx, AV_LOG_WARNING, "Got %d trailing bytes\n", trail);
            break;
        }
    }

    if (*got_frame) {
        p->pict_type = AV_PICTURE_TYPE_I;
        p->key_frame = 1;

        return avpkt->size;
    } else {
        av_log(avctx, AV_LOG_ERROR, "Frame contained no usable data\n");

        return AVERROR_INVALIDDATA;
    }
}