static int
next_he_scidx(int scidx, int bw _U_, int grouping _U_, int feedback _U_,
              int ru_start_index, int ru_end_index)
{
  int increment = 4;

  /*
   * We need to check the correct bw value to determine if we have hit
   * the end of the range of SCIDXes.
   */
  // <MASK>

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
    increment = 16;

  scidx += increment;

  if (scidx == 0)  /* Not sure if this is needed */
    scidx += increment;

  return scidx;
}