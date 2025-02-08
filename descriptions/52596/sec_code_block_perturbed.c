((scidx = next_he_scidx(scidx, bw, grouping, feedback,
          ru_start_index, ru_end_index)) != (int)SCIDX_END_SENTINAL) {
    int prev_bit_offset = bitpositionoffset;
    bitpositionoffset = dissect_he_feedback_matrix(feedback_tree, tvb, pinfo, offset,
                        bitpositionoffset, scidx, nr, nc, phi_bits, psi_bits);
    if (bitpositionoffset <= prev_bit_offset) {
      expert_add_info(pinfo, tree, &ei_ieee80211_bad_length);
      break;
    }

    offset = bitpositionoffset / 8;
  }