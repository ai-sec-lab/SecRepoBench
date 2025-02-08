static void
apply_stch (const hb_ot_shape_plan_t *plan,
	    hb_buffer_t              *buffer,
	    hb_font_t                *font)
{
  if (likely (!(buffer->scratch_flags & HB_BUFFER_SCRATCH_FLAG_ARABIC_HAS_STCH)))
    return;

  /* The Arabic shaper currently always processes in RTL mode, so we should
   * stretch / position the stretched pieces to the left / preceding glyphs. */

  /* We do a two pass implementation:
   * First pass calculates the exact number of extra glyphs we need,
   * We then enlarge buffer to have that much room,
   * Second pass applies the stretch, copying things to the end of buffer.
   */

  int sign = font->x_scale < 0 ? -1 : +1;
  unsigned int extra_glyphs_needed = 0; // Set during MEASURE, used during CUT
  typedef enum { MEASURE, CUT } step_t;

  for (step_t step = MEASURE; step <= CUT; step = (step_t) (step + 1))
  {
    unsigned int count = buffer->len;
    hb_glyph_info_t *info = buffer->info;
    hb_glyph_position_t *pos = buffer->pos;
    unsigned int new_len = count + extra_glyphs_needed; // write head during CUT
    unsigned int j = new_len;
    for (unsigned int i = count; i; i--)
    {
      if (!hb_in_range<unsigned> (info[i - 1].arabic_shaping_action(), STCH_FIXED, STCH_REPEATING))
      {
        if (step == CUT)
	{
	  --j;
	  info[j] = info[i - 1];
	  pos[j] = pos[i - 1];
	}
        continue;
      }

      /* Yay, justification! */

      hb_position_t w_total = 0; // Total to be filled
      hb_position_t w_fixed = 0; // Sum of fixed tiles
      hb_position_t w_repeating = 0; // Sum of repeating tiles
      int n_fixed = 0;
      int n_repeating = 0;

      unsigned int end = i;
      while (i &&
	     hb_in_range<unsigned> (info[i - 1].arabic_shaping_action(), STCH_FIXED, STCH_REPEATING))
      {
	i--;
	hb_position_t width = font->get_glyph_h_advance (info[i].codepoint);
	if (info[i].arabic_shaping_action() == STCH_FIXED)
	{
	  w_fixed += width;
	  n_fixed++;
	}
	else
	{
	  w_repeating += width;
	  n_repeating++;
	}
      }
      unsigned int start = i;
      unsigned int context = i;
      while (context &&
	     !hb_in_range<unsigned> (info[context - 1].arabic_shaping_action(), STCH_FIXED, STCH_REPEATING) &&
	     (_hb_glyph_info_is_default_ignorable (&info[context - 1]) ||
	      HB_ARABIC_GENERAL_CATEGORY_IS_WORD (_hb_glyph_info_get_general_category (&info[context - 1]))))
      {
	context--;
	w_total += pos[context].x_advance;
      }
      i++; // Don't touch i again.

      DEBUG_MSG (ARABIC, nullptr, "%s stretch at (%d,%d,%d)",
		 step == MEASURE ? "measuring" : "cutting", context, start, end);
      DEBUG_MSG (ARABIC, nullptr, "rest of word:    count=%d width %d", start - context, w_total);
      DEBUG_MSG (ARABIC, nullptr, "fixed tiles:     count=%d width=%d", n_fixed, w_fixed);
      DEBUG_MSG (ARABIC, nullptr, "repeating tiles: count=%d width=%d", n_repeating, w_repeating);

      /* Number of additional times to repeat each repeating tile. */
      int n_copies = 0;

      hb_position_t w_remaining = w_total - w_fixed;
      if (sign * w_remaining > sign * w_repeating && sign * w_repeating > 0)
	n_copies = (sign * w_remaining) / (sign * w_repeating) - 1;

      /* See if we can improve the fit by adding an extra repeat and squeezing them together a bit. */
      hb_position_t extra_repeat_overlap = 0;
      hb_position_t shortfall = sign * w_remaining - sign * w_repeating * (n_copies + 1);
      if (shortfall > 0 && n_repeating > 0)
      {
        ++n_copies;
        hb_position_t excess = (n_copies + 1) * sign * w_repeating - sign * w_remaining;
        if (excess > 0)
          extra_repeat_overlap = excess / (n_copies * n_repeating);
      }

      if (step == MEASURE)
      // <MASK>
    }

    if (step == MEASURE)
    {
      if (unlikely (!buffer->ensure (count + extra_glyphs_needed)))
        break;
    }
    else
    {
      assert (j == 0);
      buffer->len = new_len;
    }
  }
}