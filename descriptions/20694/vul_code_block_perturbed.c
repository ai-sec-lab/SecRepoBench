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