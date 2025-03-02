proto_tree_add_uint_format(field_tree, hf_tcp_option_sack_sre, tvb,
                                   offset+4, 4, rightedge,
                                   "right edge = %u%s", rightedge,
                                   (tcp_analyze_seq && tcp_relative_seq) ? " (relative)" : "");
        tcp_info_append_uint(pinfo, "SLE", leftedge);
        tcp_info_append_uint(pinfo, "SRE", rightedge);
        num_sack_ranges++;

        /* Store blocks for BiF analysis */
        if (tcp_analyze_seq && tcpd->fwd->tcp_analyze_seq_info && tcp_track_bytes_in_flight) {
            tcpd->fwd->tcp_analyze_seq_info->num_sack_ranges = num_sack_ranges;
            tcpd->fwd->tcp_analyze_seq_info->sack_left_edge[num_sack_ranges] = leftedge;
            tcpd->fwd->tcp_analyze_seq_info->sack_right_edge[num_sack_ranges] = rightedge;
        }

        /* Update tap info */