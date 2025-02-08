if (tvb_captured_length_remaining(tvb, offset)) {
    next_tvb = tvb_new_subset_remaining(tvb, offset);
    if (!uses_inactive_subset){
      if (dissector_try_heuristic(cotp_heur_subdissector_list, next_tvb, pinfo,
                                  tree, &hdtbl_entry, NULL)) {
        *subdissector_found = TRUE;
      } else {
        call_data_dissector(next_tvb, pinfo, tree);
      }
    }
    else
      call_data_dissector( next_tvb, pinfo, tree);
    offset += tvb_captured_length_remaining(tvb, offset);
    /* we dissected all of the containing PDU */
  }

  return offset;