hb_glyph_position_t *pos = buffer->pos;
  for (unsigned int idx = 0; idx < count;)
  {
    if (!(buffer->cur().mask & kern_mask))
    {
      idx++;
      continue;
    }

    skippy_iter.reset (idx, 1);
    if (!skippy_iter.next ())
    {
      idx++;
      continue;
    }

    hb_position_t x_kern, y_kern;
    font->get_glyph_kerning_for_direction (info[idx].codepoint,
					   info[skippy_iter.idx].codepoint,
					   buffer->props.direction,
					   &x_kern, &y_kern);

    if (x_kern)
    {
      hb_position_t kern1 = x_kern >> 1;
      hb_position_t kern2 = x_kern - kern1;
      pos[idx].x_advance += kern1;
      pos[skippy_iter.idx].x_advance += kern2;
      pos[skippy_iter.idx].x_offset += kern2;
      buffer->unsafe_to_break (idx, skippy_iter.idx + 1);
    }

    if (y_kern)
    {
      hb_position_t kern1 = y_kern >> 1;
      hb_position_t kern2 = y_kern - kern1;
      pos[idx].y_advance += kern1;
      pos[skippy_iter.idx].y_advance += kern2;
      pos[skippy_iter.idx].y_offset += kern2;
      buffer->unsafe_to_break (idx, skippy_iter.idx + 1);
    }

    idx = skippy_iter.idx;
  }