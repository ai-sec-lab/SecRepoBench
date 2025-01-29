hb_vector_t<glyf_impl::SubsetGlyph> glyphs;
    if (!_populate_subset_glyphs (context->plan, font, glyphs))
    {
      hb_font_destroy (font);
      return_trace (false);
    }

    if (font)
      hb_font_destroy (font);

    hb_vector_t<unsigned> padded_offsets;
    if (unlikely (!padded_offsets.alloc (glyphs.length, true)))
    {
      _free_compiled_subset_glyphs (glyphs);
      return_trace (false);
    }

    unsigned max_offset = 0;
    for (auto &g : glyphs)
    {
      unsigned size = g.padded_size ();
      padded_offsets.push (size);
      max_offset += size;
    }

    bool use_short_loca = false;
    if (likely (!context->plan->force_long_loca))
      use_short_loca = max_offset < 0x1FFFF;

    if (!use_short_loca)
    {
      padded_offsets.resize (0);
      for (auto &g : glyphs)
	padded_offsets.push (g.length ());
    }

    if (unlikely (!context->serializer->check_success (glyf_impl::_add_loca_and_head (context,
						 padded_offsets.iter (),
						 use_short_loca))))
    {
      _free_compiled_subset_glyphs (glyphs);
      return_trace (false);
    }

    glyf *glyf_prime = context->serializer->start_embed <glyf> ();
    if (unlikely (!glyf_prime))
    {
      _free_compiled_subset_glyphs (glyphs);
      return_trace (false);
    }

    bool result = glyf_prime->serialize (context->serializer, hb_iter (glyphs), use_short_loca, context->plan);
    if (context->plan->normalized_coords && !context->plan->pinned_at_default)
      _free_compiled_subset_glyphs (glyphs);

    return result;