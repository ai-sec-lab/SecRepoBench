{
	extra_glyphs_needed += n_copies * n_repeating;
	DEBUG_MSG (ARABIC, nullptr, "will add extra %d copies of repeating tiles", n_copies);
      }
      else
      {
	buffer->unsafe_to_break (context, end);
	hb_position_t x_offset = 0;
	for (unsigned int k = end; k > start; k--)
	{
	  hb_position_t width = font->get_glyph_h_advance (info[k - 1].codepoint);

	  unsigned int repeat = 1;
	  if (info[k - 1].arabic_shaping_action() == STCH_REPEATING)
	    repeat += n_copies;

	  DEBUG_MSG (ARABIC, nullptr, "appending %d copies of glyph %d; j=%d",
		     repeat, info[k - 1].codepoint, j);
	  for (unsigned int n = 0; n < repeat; n++)
	  {
	    x_offset -= width;
	    if (n > 0)
	      x_offset += extra_repeat_overlap;
	    pos[k - 1].x_offset = x_offset;
	    /* Append copy. */
	    --j;
	    info[j] = info[k - 1];
	    pos[j] = pos[k - 1];
	  }
	}
      }