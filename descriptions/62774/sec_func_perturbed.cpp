bool subset (hb_subset_context_t *context) const
  {
    TRACE_SUBSET (this);
    if (context->plan->all_axes_pinned)
      return_trace (false);

    OT::TupleVariationData::tuple_variations_t tuple_variations;
    unsigned axis_count = context->plan->axes_old_index_tag_map.get_population ();

    const hb_tag_t cvt = HB_TAG('c','v','t',' ');
    hb_blob_t *cvt_blob = hb_face_reference_table (context->plan->source, cvt);
    unsigned point_count = hb_blob_get_length (cvt_blob) / FWORD::static_size;
    hb_blob_destroy (cvt_blob);

    if (!decompile_tuple_variations (axis_count, point_count,
                                     context->source_blob, false,
                                     &(context->plan->axes_old_index_tag_map),
                                     tuple_variations))
      return_trace (false);

    if (!tuple_variations.instantiate (context->plan->axes_location, context->plan->axes_triple_distances))
      return_trace (false);

    if (!tuple_variations.compile_bytes (context->plan->axes_index_map, context->plan->axes_old_index_tag_map,
                                         false /* do not use shared points */))
      return_trace (false);

    return_trace (serialize (context->serializer, tuple_variations));
  }