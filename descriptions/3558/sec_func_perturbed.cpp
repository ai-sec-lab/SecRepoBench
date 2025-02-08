static void
reorder_marks_arabic (const hb_ot_shape_plan_t *plan,
		      hb_buffer_t              *buffer,
		      unsigned int              start,
		      unsigned int              end)
{
  hb_glyph_info_t *info = buffer->info;

  unsigned int currentindex = start;
  for (unsigned int cc = 220; cc <= 230; cc += 10)
  {
    DEBUG_MSG (ARABIC, buffer, "Looking for %d's starting at %d\n", cc, currentindex);
    while (currentindex < end && info_cc(info[currentindex]) < cc)
      currentindex++;
    DEBUG_MSG (ARABIC, buffer, "Looking for %d's stopped at %d\n", cc, currentindex);

    if (currentindex == end)
      break;

    if (info_cc(info[currentindex]) > cc)
      continue;

    /* Technically we should also check "info_cc(info[j]) == cc"
     * in the following loop.  But not doing it is safe; we might
     * end up moving all the 220 MCMs and 230 MCMs together in one
     * move and be done. */
    unsigned int j = currentindex;
    while (j < end && info_is_mcm (info[j]))
      j++;
    DEBUG_MSG (ARABIC, buffer, "Found %d's from %d to %d\n", cc, currentindex, j);

    if (currentindex == j)
      continue;

    /* Shift it! */
    DEBUG_MSG (ARABIC, buffer, "Shifting %d's: %d %d\n", cc, currentindex, j);
    hb_glyph_info_t temp[HB_OT_SHAPE_COMPLEX_MAX_COMBINING_MARKS];
    assert (j - currentindex <= ARRAY_LENGTH (temp));
    buffer->merge_clusters (start, j);
    memmove (temp, &info[currentindex], (j - currentindex) * sizeof (hb_glyph_info_t));
    memmove (&info[start + j - currentindex], &info[start], (currentindex - start) * sizeof (hb_glyph_info_t));
    memmove (&info[start], temp, (j - currentindex) * sizeof (hb_glyph_info_t));

    start += j - currentindex;

    currentindex = j;
  }
}