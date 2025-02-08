static void
reorder_marks_arabic (const hb_ot_shape_plan_t *plan,
		      hb_buffer_t              *buffer,
		      unsigned int              start,
		      unsigned int              end)
{
  hb_glyph_info_t *info = buffer->info;

  unsigned int i = start;
  for (unsigned int cc = 220; cc <= 230; cc += 10)
  {
    DEBUG_MSG (ARABIC, buffer, "Looking for %d's starting at %d\n", cc, i);
    while (i < end && info_cc(info[i]) < cc)
      i++;
    DEBUG_MSG (ARABIC, buffer, "Looking for %d's stopped at %d\n", cc, i);

    if (i == end)
      break;

    if (info_cc(info[i]) > cc)
      continue;

    /* Technically we should also check "info_cc(info[j]) == cc"
     * in the following loop.  But not doing it is safe; we might
     * end up moving all the 220 MCMs and 230 MCMs together in one
     * move and be done. */
    unsigned int j = i;
    while (j < end && info_is_mcm (info[j]))
      j++;
    DEBUG_MSG (ARABIC, buffer, "Found %d's from %d to %d\n", cc, i, j);

    if (i == j)
      continue;

    /* Shift it! */
    // <MASK>

    start += j - i;

    i = j;
  }
}