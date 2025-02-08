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
    // <MASK>
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