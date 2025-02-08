switch (bw) {
  case BW_20MHz:
    if (grouping == 0) {
      if (ru_end_index >= N_SCIDX_20MHZ_NG4 ||
          scidx == scidx_20MHz_Ng4[ru_end_index].end)  /* we returned the max */
        return SCIDX_END_SENTINAL;
    } else {
      if (ru_end_index >= N_SCIDX_20MHZ_NG16 ||
          scidx == scidx_20MHz_Ng16[ru_end_index].end)
        return SCIDX_END_SENTINAL;
    }
    break;
  case BW_40MHz:
    if (grouping == 0) {
      if (ru_end_index >= N_SCIDX_40MHZ_NG4 ||
          scidx == scidx_40MHz_Ng4[ru_end_index].end)
        return SCIDX_END_SENTINAL;
    } else {
      if (ru_end_index >= N_SCIDX_40MHZ_NG16 ||
          scidx == scidx_40MHz_Ng16[ru_end_index].end)
        return SCIDX_END_SENTINAL;
    }
    break;
  case BW_80MHz:
    if (grouping == 0) {
      if (ru_end_index >= N_SCIDX_80MHZ_NG4 ||
          scidx == scidx_80MHz_Ng4[ru_end_index].end)
        return SCIDX_END_SENTINAL;
    } else {
      if (ru_end_index >= N_SCIDX_80MHZ_NG16 ||
          scidx == scidx_80MHz_Ng16[ru_end_index].end)
        return SCIDX_END_SENTINAL;
    }
    break;
  case BW_160MHz:
    return SCIDX_END_SENTINAL;
  }

  /*
   * Check if this is the first time though and figure out the starting
   * SCIDX.
   */
  if (scidx == (int)SCIDX_END_SENTINAL)
    switch (bw) {
    case BW_20MHz:
      if (grouping == 0) {
        if (ru_start_index >= N_SCIDX_20MHZ_NG4)
          return SCIDX_END_SENTINAL;
        else
          return scidx_20MHz_Ng4[ru_start_index].start;
      } else {
        if (ru_start_index >= N_SCIDX_20MHZ_NG16)
          return SCIDX_END_SENTINAL;
        else
          return scidx_20MHz_Ng16[ru_start_index].start;
      }
    case BW_40MHz:
      if (grouping == 0) {
        if (ru_start_index >= N_SCIDX_40MHZ_NG4)
          return SCIDX_END_SENTINAL;
        else
          return scidx_40MHz_Ng4[ru_start_index].start;
      } else {
        if (ru_start_index >= N_SCIDX_40MHZ_NG16)
          return SCIDX_END_SENTINAL;
        else
          return scidx_40MHz_Ng16[ru_start_index].start;
      }
    case BW_80MHz:
      if (grouping == 0) {
        if (ru_start_index >= N_SCIDX_80MHZ_NG4)
          return SCIDX_END_SENTINAL;
        else
          return scidx_80MHz_Ng4[ru_start_index].start;
      } else {
        if (ru_start_index >= N_SCIDX_80MHZ_NG16)
          return SCIDX_END_SENTINAL;
        else
          return scidx_80MHz_Ng16[ru_start_index].start;
      }
    case BW_160MHz:
      return SCIDX_END_SENTINAL;
  }