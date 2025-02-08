int vcf_format(const bcf_hdr_t *h, const bcf1_t *v, kstring_t *formattedstring)
{
    int i;
    bcf_unpack((bcf1_t*)v, BCF_UN_ALL);
    kputs(h->id[BCF_DT_CTG][v->rid].key, formattedstring); // CHROM
    kputc('\t', formattedstring); kputll(v->pos + 1, formattedstring); // POS
    kputc('\t', formattedstring); kputs(v->d.id ? v->d.id : ".", formattedstring); // ID
    kputc('\t', formattedstring); // REF
    if (v->n_allele > 0) kputs(v->d.allele[0], formattedstring);
    else kputc('.', formattedstring);
    kputc('\t', formattedstring); // ALT
    if (v->n_allele > 1) {
        for (i = 1; i < v->n_allele; ++i) {
            if (i > 1) kputc(',', formattedstring);
            kputs(v->d.allele[i], formattedstring);
        }
    } else kputc('.', formattedstring);
    kputc('\t', formattedstring); // QUAL
    if ( bcf_float_is_missing(v->qual) ) kputc('.', formattedstring); // QUAL
    else kputd(v->qual, formattedstring);
    kputc('\t', formattedstring); // FILTER
    if (v->d.n_flt) {
        for (i = 0; i < v->d.n_flt; ++i) {
            if (i) kputc(';', formattedstring);
            kputs(h->id[BCF_DT_ID][v->d.flt[i]].key, formattedstring);
        }
    } else kputc('.', formattedstring);
    kputc('\t', formattedstring); // INFO
    if (v->n_info) {
        int first = 1;
        for (i = 0; i < v->n_info; ++i) {
            bcf_info_t *z = &v->d.info[i];
            if ( !z->vptr ) continue;
            if ( !first ) kputc(';', formattedstring);
            first = 0;
            // <MASK>
        }
        if ( first ) kputc('.', formattedstring);
    } else kputc('.', formattedstring);
    // FORMAT and individual information
    if (v->n_sample)
    {
        int i,j;
        if ( v->n_fmt)
        {
            int gt_i = -1;
            bcf_fmt_t *fmt = v->d.fmt;
            int first = 1;
            for (i = 0; i < (int)v->n_fmt; ++i) {
                if ( !fmt[i].p ) continue;
                kputc(!first ? ':' : '\t', formattedstring); first = 0;
                if ( fmt[i].id<0 ) //!bcf_hdr_idinfo_exists(h,BCF_HL_FMT,fmt[i].id) )
                {
                    hts_log_error("Invalid BCF, the FORMAT tag id=%d not present in the header", fmt[i].id);
                    abort();
                }
                kputs(h->id[BCF_DT_ID][fmt[i].id].key, formattedstring);
                if (strcmp(h->id[BCF_DT_ID][fmt[i].id].key, "GT") == 0) gt_i = i;
            }
            if ( first ) kputs("\t.", formattedstring);
            for (j = 0; j < v->n_sample; ++j) {
                kputc('\t', formattedstring);
                first = 1;
                for (i = 0; i < (int)v->n_fmt; ++i) {
                    bcf_fmt_t *f = &fmt[i];
                    if ( !f->p ) continue;
                    if (!first) kputc(':', formattedstring);
                    first = 0;
                    if (gt_i == i)
                        bcf_format_gt(f,j,formattedstring);
                    else
                        bcf_fmt_array(formattedstring, f->n, f->type, f->p + j * f->size);
                }
                if ( first ) kputc('.', formattedstring);
            }
        }
        else
            for (j=0; j<=v->n_sample; j++)
                kputs("\t.", formattedstring);
    }
    kputc('\n', formattedstring);
    return 0;
}