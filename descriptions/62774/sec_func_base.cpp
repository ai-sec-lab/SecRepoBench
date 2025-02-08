bool subset (hb_subset_context_t *c) const
  {
    TRACE_SUBSET (this);
    if (c->plan->all_axes_pinned)
      return_trace (false);

    OT::TupleVariationData::tuple_variations_t tuple_variations;
    unsigned axis_count = c->plan->axes_old_index_tag_map.get_population ();

    const hb_tag_t cvt = HB_TAG('c','v','t',' ');
    hb_blob_t *cvt_blob = hb_face_reference_table (c->plan->source, cvt);
    unsigned point_count = hb_blob_get_length (cvt_blob) / FWORD::static_size;
    hb_blob_destroy (cvt_blob);

    if (!decompile_tuple_variations (axis_count, point_count,
                                     c->source_blob, false,
                                     &(c->plan->axes_old_index_tag_map),
                                     tuple_variations))
      return_trace (false);

    if (!tuple_variations.instantiate (c->plan->axes_location, c->plan->axes_triple_distances))
      return_trace (false);

    if (!tuple_variations.compile_bytes (c->plan->axes_index_map, c->plan->axes_old_index_tag_map,
                                         false /* do not use shared points */))
      return_trace (false);

    return_trace (serialize (c->serializer, tuple_variations));
  }