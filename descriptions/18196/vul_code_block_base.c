if (z->key >= h->n[BCF_DT_ID]) {
                hts_log_error("Invalid BCF, the INFO index is too large");
                errno = EINVAL;
                return -1;
            }
            kputs(h->id[BCF_DT_ID][z->key].key, s);
            if (z->len <= 0) continue;
            kputc('=', s);
            if (z->len == 1)
            {
                switch (z->type)
                {
                    case BCF_BT_INT8:  if ( z->v1.i==bcf_int8_missing ) kputc('.', s); else kputw(z->v1.i, s); break;
                    case BCF_BT_INT16: if ( z->v1.i==bcf_int16_missing ) kputc('.', s); else kputw(z->v1.i, s); break;
                    case BCF_BT_INT32: if ( z->v1.i==bcf_int32_missing ) kputc('.', s); else kputw(z->v1.i, s); break;
                    case BCF_BT_INT64: if ( z->v1.i==bcf_int64_missing ) kputc('.', s); else kputll(z->v1.i, s); break;
                    case BCF_BT_FLOAT: if ( bcf_float_is_missing(z->v1.f) ) kputc('.', s); else kputd(z->v1.f, s); break;
                    case BCF_BT_CHAR:  kputc(z->v1.i, s); break;
                    default: hts_log_error("Unexpected type %d", z->type); exit(1); break;
                }
            }
            else bcf_fmt_array(s, z->len, z->type, z->vptr);