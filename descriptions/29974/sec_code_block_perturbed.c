num_sack_ranges++;

        /* Store blocks for BiF analysis */
        if (tcp_analyze_seq && tcpd->fwd->tcp_analyze_seq_info && tcp_track_bytes_in_flight && num_sack_ranges < MAX_TCP_SACK_RANGES) {
            tcpd->fwd->tcp_analyze_seq_info->num_sack_ranges = num_sack_ranges;
            tcpd->fwd->tcp_analyze_seq_info->sack_left_edge[num_sack_ranges] = leftedge;
            tcpd->fwd->tcp_analyze_seq_info->sack_right_edge[num_sack_ranges] = rightedge;
        }