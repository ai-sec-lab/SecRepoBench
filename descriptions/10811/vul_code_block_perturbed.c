switch (bw) {
  case BW_20MHz:
    if (grouping == 0) {
      if (scidx == scidx_20MHz_Ng4[ru_end_index].end)  /* we returned the max */
        return SCIDX_END_SENTINAL;
    } else {
      if (scidx == scidx_20MHz_Ng16[ru_end_index].end)
        return SCIDX_END_SENTINAL;
    }
    break;
  case BW_40MHz:
    if (grouping == 0) {
      if (scidx == scidx_40MHz_Ng4[ru_end_index].end)
        return SCIDX_END_SENTINAL;
    } else {
      if (scidx == scidx_40MHz_Ng16[ru_end_index].end)
        return SCIDX_END_SENTINAL;
    }
    break;
  case BW_80MHz:
    if (grouping == 0) {
      if (scidx == scidx_80MHz_Ng4[ru_end_index].end)
        return SCIDX_END_SENTINAL;
    } else {
      if (scidx == scidx_80MHz_Ng16[ru_end_index].end)
        return SCIDX_END_SENTINAL;
    }
    break;
  case BW_160MHz:
    return SCIDX_END_SENTINAL;
  }

  /*
 *    * Check if this is the first time though and figure out the starting
 *       * SCIDX.
 *          */
  if (scidx == (int)SCIDX_END_SENTINAL)
    switch (bw) {
    case BW_20MHz:
      if (grouping == 0)
        return scidx_20MHz_Ng4[ru_start_index].start;
      else
        return scidx_20MHz_Ng16[ru_start_index].start;
    case BW_40MHz:
      if (grouping == 0)
        return scidx_40MHz_Ng4[ru_start_index].start;
      else
        return scidx_40MHz_Ng16[ru_start_index].start;
    case BW_80MHz:
      if (grouping == 0)
        return scidx_80MHz_Ng4[ru_start_index].start;
      else
        return scidx_80MHz_Ng16[ru_start_index].start;;
    case BW_160MHz:
      return SCIDX_END_SENTINAL;
  }