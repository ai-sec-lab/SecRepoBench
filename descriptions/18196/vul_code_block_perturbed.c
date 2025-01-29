if (z->key >= h->n[BCF_DT_ID]) {
                hts_log_error("Invalid BCF, the INFO index is too large");
                errno = EINVAL;
                return -1;
            }
            kputs(h->id[BCF_DT_ID][z->key].key, formattedstring);
            if (z->len <= 0) continue;
            kputc('=', formattedstring);
            if (z->len == 1)
            {
                switch (z->type)
                {
                    case BCF_BT_INT8:  if ( z->v1.i==bcf_int8_missing ) kputc('.', formattedstring); else kputw(z->v1.i, formattedstring); break;
                    case BCF_BT_INT16: if ( z->v1.i==bcf_int16_missing ) kputc('.', formattedstring); else kputw(z->v1.i, formattedstring); break;
                    case BCF_BT_INT32: if ( z->v1.i==bcf_int32_missing ) kputc('.', formattedstring); else kputw(z->v1.i, formattedstring); break;
                    case BCF_BT_INT64: if ( z->v1.i==bcf_int64_missing ) kputc('.', formattedstring); else kputll(z->v1.i, formattedstring); break;
                    case BCF_BT_FLOAT: if ( bcf_float_is_missing(z->v1.f) ) kputc('.', formattedstring); else kputd(z->v1.f, formattedstring); break;
                    case BCF_BT_CHAR:  kputc(z->v1.i, formattedstring); break;
                    default: hts_log_error("Unexpected type %d", z->type); exit(1); break;
                }
            }
            else bcf_fmt_array(formattedstring, z->len, z->type, z->vptr);