((scidx = next_he_scidx(scidx, bw, grouping, feedback,
          ru_start_index, ru_end_index)) != (int)SCIDX_END_SENTINAL) {
    int prev_bit_offset = bit_offset;
    bit_offset = dissect_he_feedback_matrix(feedback_tree, tvb, pinfo, offset,
                        bit_offset, scidx, nr, nc, phi_bits, psi_bits);
    if (bit_offset <= prev_bit_offset) {
      expert_add_info(pinfo, tree, &ei_ieee80211_bad_length);
      break;
    }

    offset = bit_offset / 8;
  }