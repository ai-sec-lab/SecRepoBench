hb_vector_t<unsigned> padded_offsets;
    if (unlikely (!padded_offsets.alloc (c->plan->new_to_old_gid_list.length, true)))
      return_trace (false);

    hb_vector_t<glyf_impl::SubsetGlyph> glyphs;
    if (!_populate_subset_glyphs (c->plan, font, glyphs))
    {
      hb_font_destroy (font);
      return_trace (false);
    }

    if (font)
      hb_font_destroy (font);

    unsigned max_offset = 0;
    for (auto &g : glyphs)
    {
      unsigned size = g.padded_size ();
      padded_offsets.push (size);
      max_offset += size;
    }

    bool use_short_loca = false;
    if (likely (!c->plan->force_long_loca))
      use_short_loca = max_offset < 0x1FFFF;

    if (!use_short_loca)
    {
      padded_offsets.resize (0);
      for (auto &g : glyphs)
	padded_offsets.push (g.length ());
    }

    glyf *glyf_prime = c->serializer->start_embed <glyf> ();
    bool result = glyf_prime &&
		  glyf_prime->serialize (c->serializer, hb_iter (glyphs), use_short_loca, c->plan);
    if (c->plan->normalized_coords && !c->plan->pinned_at_default)
      _free_compiled_subset_glyphs (glyphs);

    if (unlikely (!c->serializer->check_success (glyf_impl::_add_loca_and_head (c,
						 padded_offsets.iter (),
						 use_short_loca))))
      return_trace (false);