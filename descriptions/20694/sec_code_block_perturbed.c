cp += safe_itf8_get((char *)cp,  (char *)cplimit, &hdr->ref_seq_id);
#ifdef LARGE_POS
        cp += safe_ltf8_get((char *)cp,  (char *)cplimit, &hdr->ref_seq_start);
        cp += safe_ltf8_get((char *)cp,  (char *)cplimit, &hdr->ref_seq_span);
#else
        int32_t i32;
        cp += safe_itf8_get((char *)cp,  (char *)cplimit, &i32);
        hdr->ref_seq_start = i32;
        cp += safe_itf8_get((char *)cp,  (char *)cplimit, &i32);
        hdr->ref_seq_span = i32;
#endif
        if (hdr->ref_seq_start < 0 || hdr->ref_seq_span < 0) {
            free(hdr);
            hts_log_error("Negative values not permitted for header "
                          "sequence start or span fields");
            return NULL;
        }