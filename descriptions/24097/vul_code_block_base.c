if (align_mem(mem) < 0) {
            hts_log_error("Memory allocation failure at %s:%"PRIhts_pos, bcf_seqname_safe(h,v), v->pos+1);
            v->errcode |= BCF_ERR_LIMITS;
            return -1;
        }
        f->offset = mem->l;

        // Limit the total memory to ~2Gb per VCF row.  This should mean
        // malformed VCF data is less likely to take excessive memory and/or
        // time.
        if (v->n_sample * (uint64_t)f->size > INT_MAX) {
            hts_log_error("Excessive memory required by FORMAT fields at %s:%"PRIhts_pos, bcf_seqname_safe(h,v), v->pos+1);
            v->errcode |= BCF_ERR_LIMITS;
            return -1;
        }