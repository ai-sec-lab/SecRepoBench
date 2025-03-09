slice->ref = (char *)BLOCK_DATA(b);
            slice->ref_start = slice->hdr->ref_seq_start;
            slice->ref_end   = slice->hdr->ref_seq_start + slice->hdr->ref_seq_span-1;
            if (slice->hdr->ref_seq_span > b->uncomp_size) {
                hts_log_error("Embedded reference is too small at #%d:%d-%d",
                              ref_id, slice->ref_start, slice->ref_end);
                return -1;
            }