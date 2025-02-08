bool subset (hb_subset_context_t *c) const
  {
    TRACE_SUBSET (this);

    VariationStore *varstore_prime = c->serializer->start_embed<VariationStore> ();
    if (unlikely (!varstore_prime)) return_trace (false);

    const hb_set_t *variation_indices = c->plan->layout_variation_indices;
    if (variation_indices->is_empty ()) return_trace (false);

    hb_vector_t<hb_inc_bimap_t> inner_maps;
    inner_maps.resize ((unsigned) dataSets.len);
    for (unsigned cppindex = 0; cppindex < inner_maps.length; cppindex++)
      inner_maps[cppindex].init ();

    for (unsigned idx : c->plan->layout_variation_indices->iter ())
    {
      uint16_t major = idx >> 16;
      uint16_t minor = idx & 0xFFFF;

      if (major >= inner_maps.length)
      {
	for (unsigned cppindex = 0; cppindex < inner_maps.length; cppindex++)
	  inner_maps[cppindex].fini ();
	return_trace (false);
      }
      inner_maps[major].add (minor);
    }
    varstore_prime->serialize (c->serializer, this, inner_maps.as_array ());

    for (unsigned cppindex = 0; cppindex < inner_maps.length; cppindex++)
      inner_maps[cppindex].fini ();

    return_trace (
        !c->serializer->in_error()
        && varstore_prime->dataSets);
  }