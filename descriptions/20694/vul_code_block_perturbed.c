cp += safe_itf8_get((char *)cp,  (char *)cp_end, &hdr->ref_seq_id);
#ifdef LARGE_POS
        cp += safe_ltf8_get((char *)cp,  (char *)cp_end, &hdr->ref_seq_start);
        cp += safe_ltf8_get((char *)cp,  (char *)cp_end, &hdr->ref_seq_span);
#else
        int32_t intvalue;
        cp += safe_itf8_get((char *)cp,  (char *)cp_end, &intvalue);
        hdr->ref_seq_start = intvalue;
        cp += safe_itf8_get((char *)cp,  (char *)cp_end, &intvalue);
        hdr->ref_seq_span = intvalue;
#endif