bool subset (hb_subset_context_t* context) const
  {
    TRACE_SUBSET (this);

    sbix *sbix_prime = context->serializer->start_embed<sbix> ();
    if (unlikely (!sbix_prime)) return_trace (false);
    if (unlikely (!context->serializer->embed (this->version))) return_trace (false);
    if (unlikely (!context->serializer->embed (this->flags))) return_trace (false);
    hb_blob_ptr_t<sbix> table = hb_sanitize_context_t().reference_table<sbix> (context->plan->source);
    const unsigned int sbix_len = table.get_blob ()->length;
    table.destroy ();

    return_trace (serialize_strike_offsets (context, sbix_prime, sbix_len));
  }