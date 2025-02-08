bool serialize (hb_serialize_context_t *c,
		  const VarData *src,
		  const hb_inc_bimap_t &inner_map,
		  const hb_bimap_t &region_map)
  {
    TRACE_SERIALIZE (this);
    if (unlikely (!c->extend_min (this))) return_trace (false);
    itemCount = inner_map.get_next_value ();

    /* Optimize word count */
    unsigned ri_count = src->regionIndices.len;
    enum delta_size_t { kZero=0, kNonWord, kWord };
    hb_vector_t<delta_size_t> delta_sz;
    hb_vector_t<unsigned int> ri_map;	/* maps new index to old index */
    delta_sz.resize (ri_count);
    ri_map.resize (ri_count);
    unsigned int new_word_count = 0;
    unsigned int r;

    const HBUINT8 *src_delta_bytes = src->get_delta_bytes ();
    unsigned src_row_size = src->get_row_size ();
    unsigned src_word_count = src->wordCount ();
    unsigned src_long_words = src->longWords ();

    bool has_long = false;
    if (src_long_words)
    {
      for (r = 0; r < src_word_count; r++)
      {
	for (unsigned int i = 0; i < inner_map.get_next_value (); i++)
	{
	  unsigned int old = inner_map.backward (i);
	  int32_t delta = src->get_item_delta_fast (old, r, src_delta_bytes, src_row_size);
	  if (delta < -65536 || 65535 < delta)
	  {
	    has_long = true;
	    break;
	  }
        }
      }
    }

    signed min_threshold = has_long ? -65536 : -128;
    signed max_threshold = has_long ? +65535 : +127;
    for (r = 0; r < ri_count; r++)
    {
      bool short_circuit = src_long_words == has_long && src_word_count <= r;

      delta_sz[r] = kZero;
      for (unsigned int i = 0; i < inner_map.get_next_value (); i++)
      {
	unsigned int old = inner_map.backward (i);
	int32_t delta = src->get_item_delta_fast (old, r, src_delta_bytes, src_row_size);
	if (delta < min_threshold || max_threshold < delta)
	{
	  delta_sz[r] = kWord;
	  new_word_count++;
	  break;
	}
	else if (delta != 0)
	{
	  delta_sz[r] = kNonWord;
	  if (short_circuit)
	    break;
	}
      }
    }

    unsigned int word_index = 0;
    unsigned int non_word_index = new_word_count;
    unsigned int new_ri_count = 0;
    for (r = 0; r < ri_count; r++)
      if (delta_sz[r])
      {
	unsigned new_r = (delta_sz[r] == kWord)? word_index++ : non_word_index++;
	ri_map[new_r] = r;
	new_ri_count++;
      }

    wordSizeCount = new_word_count | (has_long ? 0x8000u /* LONG_WORDS */ : 0);

    regionIndices.len = new_ri_count;

    if (unlikely (!c->extend (this))) return_trace (false);

    for (r = 0; r < new_ri_count; r++)
      regionIndices[r] = region_map[src->regionIndices[ri_map[r]]];

    HBUINT8 *delta_bytes = get_delta_bytes ();
    unsigned row_size = get_row_size ();
    unsigned count = itemCount;
    for (unsigned int i = 0; i < count; i++)
    {
      unsigned int old = inner_map.backward (i);
      for (unsigned int r = 0; r < new_ri_count; r++)
	set_item_delta_fast (i, r,
			     src->get_item_delta_fast (old, ri_map[r],
						       src_delta_bytes, src_row_size),
			     delta_bytes, row_size);
    }

    return_trace (true);
  }