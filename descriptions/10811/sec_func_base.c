static int
next_he_scidx(int scidx, int bw _U_, int grouping _U_, int feedback _U_,
              int ru_start_index, int ru_end_index)
{
  int incr = 4;

  /*
   * We need to check the correct bw value to determine if we have hit
   * the end of the range of SCIDXes.
   */
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

  /*
   * for BW_20MHz it is more complex, and the start and end sets have an
   * increment of 2, and around zero they go -4, -2, 2, 4 as well.
   */
  if (bw == BW_20MHz) {
    if (grouping == 0) {
      if (scidx == -122)
        return -120;
      if (scidx == -4)
        return -2;
      if (scidx == 2)
        return 4;
      if (scidx == 120)
        return 122;
    } else {
      if (scidx == -122)
        return -116;
      if (scidx == -4)
        return -2;
      if (scidx == -2)
        return 2;
      if (scidx == 2)
        return 4;
      if (scidx == 116)
        return 122;
    }
  }

  if (grouping == 1)
    incr = 16;

  scidx += incr;

  if (scidx == 0)  /* Not sure if this is needed */
    scidx += incr;

  return scidx;
}