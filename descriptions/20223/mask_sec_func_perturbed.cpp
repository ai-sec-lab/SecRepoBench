bool subset (hb_subset_context_t* context) const
  {
    TRACE_SUBSET (this);

    sbix *sbix_prime = context->serializer->start_embed<sbix> ();
    if (unlikely (!sbix_prime)) return_trace (false);
    if (unlikely (!context->serializer->embed (this->version))) return_trace (false);
    if (unlikely (!context->serializer->embed (this->flags))) return_trace (false);
    // <MASK>
  }