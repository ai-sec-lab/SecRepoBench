static int jpeg2000_read_main_headers(Jpeg2000DecoderContext *decodercontext)
{
    Jpeg2000CodingStyle *codsty = decodercontext->codsty;
    Jpeg2000QuantStyle *qntsty  = decodercontext->qntsty;
    Jpeg2000POC         *poc    = &decodercontext->poc;
    uint8_t *properties         = decodercontext->properties;

    for (;;) {
        int len, ret = 0;
        uint16_t marker;
        int oldpos;

        if (bytestream2_get_bytes_left(&decodercontext->g) < 2) {
            av_log(decodercontext->avctx, AV_LOG_ERROR, "Missing EOC\n");
            break;
        }

        marker = bytestream2_get_be16u(&decodercontext->g);
        oldpos = bytestream2_tell(&decodercontext->g);
        if (marker >= 0xFF30 && marker <= 0xFF3F)
            continue;
        if (marker == JPEG2000_SOD) {
            Jpeg2000Tile *tile;
            Jpeg2000TilePart *tp;

            if (!decodercontext->tile) {
                av_log(decodercontext->avctx, AV_LOG_ERROR, "Missing SIZ\n");
                return AVERROR_INVALIDDATA;
            }
            if (decodercontext->curtileno < 0) {
                av_log(decodercontext->avctx, AV_LOG_ERROR, "Missing SOT\n");
                return AVERROR_INVALIDDATA;
            }

            tile = decodercontext->tile + decodercontext->curtileno;
            tp = tile->tile_part + tile->tp_idx;
            if (tp->tp_end < decodercontext->g.buffer) {
                av_log(decodercontext->avctx, AV_LOG_ERROR, "Invalid tpend\n");
                return AVERROR_INVALIDDATA;
            }

            if (decodercontext->has_ppm) {
                // Extract the size of the tile-part header from the packed headers stream.
                // Read the next 32 bits as an unsigned integer to determine the size.
                // <MASK>
                bytestream2_init(&tp->header_tpg, decodercontext->packed_headers_stream.buffer, tp_header_size);
                bytestream2_skip(&decodercontext->packed_headers_stream, tp_header_size);
            }
            if (tile->has_ppt && tile->tp_idx == 0) {
                bytestream2_init(&tile->packed_headers_stream, tile->packed_headers, tile->packed_headers_size);
            }

            bytestream2_init(&tp->tpg, decodercontext->g.buffer, tp->tp_end - decodercontext->g.buffer);
            bytestream2_skip(&decodercontext->g, tp->tp_end - decodercontext->g.buffer);

            continue;
        }
        if (marker == JPEG2000_EOC)
            break;

        len = bytestream2_get_be16(&decodercontext->g);
        if (len < 2 || bytestream2_get_bytes_left(&decodercontext->g) < len - 2) {
            if (decodercontext->avctx->strict_std_compliance >= FF_COMPLIANCE_STRICT) {
                av_log(decodercontext->avctx, AV_LOG_ERROR, "Invalid len %d left=%d\n", len, bytestream2_get_bytes_left(&decodercontext->g));
                return AVERROR_INVALIDDATA;
            }
            av_log(decodercontext->avctx, AV_LOG_WARNING, "Missing EOC Marker.\n");
            break;
        }

        switch (marker) {
        case JPEG2000_SIZ:
            if (decodercontext->ncomponents) {
                av_log(decodercontext->avctx, AV_LOG_ERROR, "Duplicate SIZ\n");
                return AVERROR_INVALIDDATA;
            }
            ret = get_siz(decodercontext);
            if (!decodercontext->tile)
                decodercontext->numXtiles = decodercontext->numYtiles = 0;
            break;
        case JPEG2000_COC:
            ret = get_coc(decodercontext, codsty, properties);
            break;
        case JPEG2000_COD:
            ret = get_cod(decodercontext, codsty, properties);
            break;
        case JPEG2000_RGN:
            ret = get_rgn(decodercontext, len);
            break;
        case JPEG2000_QCC:
            ret = get_qcc(decodercontext, len, qntsty, properties);
            break;
        case JPEG2000_QCD:
            ret = get_qcd(decodercontext, len, qntsty, properties);
            break;
        case JPEG2000_POC:
            ret = get_poc(decodercontext, len, poc);
            break;
        case JPEG2000_SOT:
            if (!decodercontext->in_tile_headers) {
                decodercontext->in_tile_headers = 1;
                if (decodercontext->has_ppm) {
                    bytestream2_init(&decodercontext->packed_headers_stream, decodercontext->packed_headers, decodercontext->packed_headers_size);
                }
            }
            if (!(ret = get_sot(decodercontext, len))) {
                av_assert1(decodercontext->curtileno >= 0);
                codsty = decodercontext->tile[decodercontext->curtileno].codsty;
                qntsty = decodercontext->tile[decodercontext->curtileno].qntsty;
                poc    = &decodercontext->tile[decodercontext->curtileno].poc;
                properties = decodercontext->tile[decodercontext->curtileno].properties;
            }
            break;
        case JPEG2000_PLM:
            // the PLM marker is ignored
        case JPEG2000_COM:
            // the comment is ignored
            bytestream2_skip(&decodercontext->g, len - 2);
            break;
        case JPEG2000_CRG:
            ret = read_crg(decodercontext, len);
            break;
        case JPEG2000_TLM:
            // Tile-part lengths
            ret = get_tlm(decodercontext, len);
            break;
        case JPEG2000_PLT:
            // Packet length, tile-part header
            ret = get_plt(decodercontext, len);
            break;
        case JPEG2000_PPM:
            // Packed headers, main header
            if (decodercontext->in_tile_headers) {
                av_log(decodercontext->avctx, AV_LOG_ERROR, "PPM Marker can only be in Main header\n");
                return AVERROR_INVALIDDATA;
            }
            ret = get_ppm(decodercontext, len);
            break;
        case JPEG2000_PPT:
            // Packed headers, tile-part header
            if (decodercontext->has_ppm) {
                av_log(decodercontext->avctx, AV_LOG_ERROR,
                       "Cannot have both PPT and PPM marker.\n");
                return AVERROR_INVALIDDATA;
            }

            ret = get_ppt(decodercontext, len);
            break;
        default:
            av_log(decodercontext->avctx, AV_LOG_ERROR,
                   "unsupported marker 0x%.4"PRIX16" at pos 0x%X\n",
                   marker, bytestream2_tell(&decodercontext->g) - 4);
            bytestream2_skip(&decodercontext->g, len - 2);
            break;
        }
        if (bytestream2_tell(&decodercontext->g) - oldpos != len || ret) {
            av_log(decodercontext->avctx, AV_LOG_ERROR,
                   "error during processing marker segment %.4"PRIx16"\n",
                   marker);
            return ret ? ret : -1;
        }
    }
    return 0;
}