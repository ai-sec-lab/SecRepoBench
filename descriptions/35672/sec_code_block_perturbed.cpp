out->paletteFlagsZ.serialize_copy (c, paletteFlagsZ, base, 0, hb_serialize_context_t::Head, numpalettes);
    out->paletteLabelsZ.serialize_copy (c, paletteLabelsZ, base, 0, hb_serialize_context_t::Head, numpalettes);

    const hb_array_t<const NameID> colorLabels = (base+colorLabelsZ).as_array (color_count);
    if (colorLabelsZ)
    {
      c->push ();
      for (const auto _ : colorLabels)
      {
        if (!color_index_map->has (_)) continue;
        NameID new_color_idx;
        new_color_idx = color_index_map->get (_);
        if (!c->copy<NameID> (new_color_idx))
        {
          c->pop_discard ();
          return_trace (false);
        }
      }
      c->add_link (out->colorLabelsZ, c->pop_pack ());
    }
    return_trace (true);