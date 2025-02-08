HB_INTERNAL bool postV2Tail::subset (hb_subset_context_t *c) const
{
  TRACE_SUBSET (this);

  const hb_map_t &reverse_glyph_map = *c->plan->reverse_glyph_map;
  unsigned num_glyphs = c->plan->num_output_glyphs ();
  hb_map_t indextranslationmap, old_gid_new_index_map;
  unsigned i = 0;

  post::accelerator_t _post (c->plan->source);

  hb_hashmap_t<hb_bytes_t, unsigned, true> glyph_name_to_new_index;
  for (hb_codepoint_t new_gid = 0; new_gid < num_glyphs; new_gid++)
  {
    hb_codepoint_t old_gid = reverse_glyph_map.get (new_gid);
    unsigned old_index = glyphNameIndex[old_gid];

    unsigned new_index;
    const unsigned *new_index2;
    if (old_index <= 257) new_index = old_index;
    else if (indextranslationmap.has (old_index, &new_index2))
    {
      new_index = *new_index2;
    } else {
      hb_bytes_t s = _post.find_glyph_name (old_gid);
      new_index = glyph_name_to_new_index.get (s);
      if (new_index == (unsigned)-1)
      {
        int standard_glyph_index = -1;
        for (unsigned i = 0; i < format1_names_length; i++)
        {
          if (s == format1_names (i))
          {
            standard_glyph_index = i;
            break;
          }
        }

        if (standard_glyph_index == -1)
        {
          new_index = 258 + i;
          i++;
        }
        else
        { new_index = standard_glyph_index; }
        glyph_name_to_new_index.set (s, new_index);
      }
      indextranslationmap.set (old_index, new_index);
    }
    old_gid_new_index_map.set (old_gid, new_index);
  }

  auto index_iter =
  + hb_range (num_glyphs)
  | hb_map (reverse_glyph_map)
  | hb_map_retains_sorting ([&](hb_codepoint_t old_gid)
                            {
                              unsigned new_index = old_gid_new_index_map.get (old_gid);
                              return hb_pair_t<unsigned, unsigned> (old_gid, new_index);
                            })
  ;

  return_trace (serialize (c->serializer, index_iter, &_post));
}