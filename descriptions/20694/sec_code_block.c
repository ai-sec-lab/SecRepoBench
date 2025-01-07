if (hdr->ref_seq_start < 0 || hdr->ref_seq_span < 0) {
            free(hdr);
            hts_log_error("Negative values not permitted for header "
                          "sequence start or span fields");
            return NULL;
        }