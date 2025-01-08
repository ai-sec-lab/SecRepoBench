if (s->hdr->ref_seq_span > b->uncomp_size) {
                hts_log_error("Embedded reference is too small at #%d:%d-%d",
                              ref_id, s->ref_start, s->ref_end);
                return -1;
            }