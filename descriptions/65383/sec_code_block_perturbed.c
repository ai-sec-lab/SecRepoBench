case 'H': {
            if (aux_limit - aux < 3)
                goto err;

            if (!tm->blk) {
                if (!(tm->blk = cram_new_block(EXTERNAL, key)))
                    goto err;
                codec->out = tm->blk;
            }

            char *aux_s;
            aux += 3;
            aux_s = aux;
            while (aux < aux_limit && *aux++);
            if (codec->encode(s, codec, aux_s, aux - aux_s) < 0)
                goto err;
            break;
        }

        case 'B': {
            if (aux_limit - aux < 4+4)
                goto err;

            int type = aux[3], blen;
            uint32_t count = (((uint32_t)((unsigned char *)aux)[4]) << 0 |
                              ((uint32_t)((unsigned char *)aux)[5]) << 8 |
                              ((uint32_t)((unsigned char *)aux)[6]) <<16 |
                              ((uint32_t)((unsigned char *)aux)[7]) <<24);
            if (!tm->blk) {
                if (!(tm->blk = cram_new_block(EXTERNAL, key)))
                    goto err;
                if (codec->u.e_byte_array_len.val_codec->codec == E_XDELTA) {
                    if (!(tm->blk2 = cram_new_block(EXTERNAL, key+128)))
                        goto err;
                    codec->u.e_byte_array_len.len_codec->out = tm->blk2;
                    codec->u.e_byte_array_len.val_codec->u.e_xdelta.sub_codec->out = tm->blk;
                } else {
                    codec->u.e_byte_array_len.len_codec->out = tm->blk;
                    codec->u.e_byte_array_len.val_codec->out = tm->blk;
                }
            }

            // skip TN field
            aux+=3;

            // We use BYTE_ARRAY_LEN with external length, so store that first
            switch (type) {
            case 'c': case 'C':
                blen = count;
                break;
            case 's': case 'S':
                blen = 2*count;
                break;
            case 'i': case 'I': case 'f':
                blen = 4*count;
                break;
            default:
                hts_log_error("Unknown sub-type '%c' for aux type 'B'", type);
                goto err;
            }

            blen += 5; // sub-type & length
            if (aux_limit - aux < blen)
                goto err;

            if (codec->encode(s, codec, aux, blen) < 0)
                goto err;
            aux += blen;
            break;
        }