void serialize (hb_serialize_context_t *c,
		  const hb_set_t *unicodes,
		  const hb_set_t *glyphs_requested,
		  const hb_map_t *glyph_map,
		  const void *base)
  {
    auto snap = c->snapshot ();
    unsigned initial_table_position = c->length ();
    const char* init_tail = c->tail;

    if (unlikely (!c->extend_min (*this))) return;
    this->format = 14;

    auto src_tbl = reinterpret_cast<const CmapSubtableFormat14*> (base);

    /*
     * Some versions of OTS require that offsets are in order. Due to the use
     * of push()/pop_pack() serializing the variation records in order results
     * in the offsets being in reverse order (first record has the largest
     * offset). While this is perfectly valid, it will cause some versions of
     * OTS to consider this table bad.
     *
     * So to prevent this issue we serialize the variation records in reverse
     * order, so that the offsets are ordered from small to large. Since
     * variation records are supposed to be in increasing order of varSelector
     * we then have to reverse the order of the written variation selector
     * records after everything is finalized.
     */
    hb_vector_t<hb_pair_t<unsigned, unsigned>> obj_indices;
    for (int i = src_tbl->record.len - 1; i >= 0; i--)
    {
      hb_pair_t<unsigned, unsigned> result = src_tbl->record[i].copy (c, unicodes, glyphs_requested, glyph_map, base);
      if (result.first || result.second)
	obj_indices.push (result);
    }

    if (c->length () - initial_table_position == CmapSubtableFormat14::min_size)
    {
      c->revert (snap);
      return;
    }

    if (unlikely (!c->check_success (!obj_indices.in_error ())))
      return;

    int tail_len = init_tail - c->tail;
    c->check_assign (this->length, c->length () - initial_table_position + tail_len);
    c->check_assign (this->record.len,
		     (c->length () - initial_table_position - CmapSubtableFormat14::min_size) /
		     VariationSelectorRecord::static_size);

    /* Correct the incorrect write order by reversing the order of the variation
       records array. */
    _reverse_variation_records ();

    /* Now that records are in the right order, we can set up the offsets. */
    _add_links_to_variation_records (c, obj_indices);
  }