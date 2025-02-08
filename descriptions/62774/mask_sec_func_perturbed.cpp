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

    // <MASK>
  }