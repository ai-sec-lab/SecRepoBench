bool subset (hb_subset_context_t *c) const
  {
    TRACE_SUBSET (this);
    // <MASK>

    switch (u.format) {
    case 1: return_trace (bool (reinterpret_cast<Anchor *> (u.format1.copy (c->serializer))));
    case 2: return_trace (bool (reinterpret_cast<Anchor *> (u.format2.copy (c->serializer))));
    case 3: return_trace (bool (reinterpret_cast<Anchor *> (u.format3.copy (c->serializer,
                                                                              c->plan->layout_variation_idx_map))));
    default:return_trace (false);
    }
  }