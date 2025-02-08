bool subset (hb_subset_context_t* c) const
  {
    TRACE_SUBSET (this);

    sbix *sbix_prime = c->serializer->start_embed<sbix> ();
    if (unlikely (!sbix_prime)) return_trace (false);
    if (unlikely (!c->serializer->embed (this->version))) return_trace (false);
    if (unlikely (!c->serializer->embed (this->flags))) return_trace (false);
    // <MASK>
  }