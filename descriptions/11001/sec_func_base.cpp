inline bool get_extents (hb_codepoint_t glyph, hb_glyph_extents_t *extents) const
    {
      unsigned int x_ppem = upem, y_ppem = upem; /* TODO Use font ppem if available. */

      if (!cblc)
	return false;  // Not a color bitmap font.

      const void *base;
      const IndexSubtableRecord *subtable_record = this->cblc->find_table (glyph, &x_ppem, &y_ppem, &base);
      if (!subtable_record || !x_ppem || !y_ppem)
	return false;

      if (subtable_record->get_extents (extents, base))
	return true;

      unsigned int image_offset = 0, image_length = 0, image_format = 0;
      if (!subtable_record->get_image_data (glyph, base, &image_offset, &image_length, &image_format))
	return false;

      {
	if (unlikely (image_offset > cbdt_len || cbdt_len - image_offset < image_length))
	  return false;

	switch (image_format)
	{
	  case 17: {
	    if (unlikely (image_length < GlyphBitmapDataFormat17::min_size))
	      return false;

	    const GlyphBitmapDataFormat17& glyphFormat17 =
		StructAtOffset<GlyphBitmapDataFormat17> (this->cbdt, image_offset);
	    glyphFormat17.glyphMetrics.get_extents (extents);
	  }
	  break;
	  default:
	    // TODO: Support other image formats.
	    return false;
	}
      }

      /* Convert to the font units. */
      extents->x_bearing *= upem / (float) x_ppem;
      extents->y_bearing *= upem / (float) y_ppem;
      extents->width *= upem / (float) x_ppem;
      extents->height *= upem / (float) y_ppem;

      return true;
    }