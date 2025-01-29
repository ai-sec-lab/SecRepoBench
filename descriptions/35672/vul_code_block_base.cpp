const hb_array_t<const HBUINT32> paletteFlags = (base+paletteFlagsZ).as_array (palette_count);
    const hb_array_t<const NameID> paletteLabels = (base+paletteLabelsZ).as_array (palette_count);
    const hb_array_t<const NameID> colorLabels = (base+colorLabelsZ).as_array (color_count);

    c->push ();
    for (const auto _ : paletteFlags)
    {
      if (!c->copy<HBUINT32> (_))
      {
        c->pop_discard ();
        return_trace (false);
      }
    }
    c->add_link (out->paletteFlagsZ, c->pop_pack ());

    c->push ();
    for (const auto _ : paletteLabels)
    {
      if (!c->copy<NameID> (_))
      {
        c->pop_discard ();
        return_trace (false);
      }
    }
    c->add_link (out->paletteLabelsZ, c->pop_pack ());

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
    return_trace (true);