glyf_table = hb_sanitize_context_t ().reference_table<glyf> (face);

      num_glyphs = hb_max (1u, loca_table.get_length () / (short_offset ? 2 : 4)) - 1;
      num_glyphs = hb_min (num_glyphs, face->get_num_glyphs ());