bool subset (hb_subset_context_t *context) const
  {
    TRACE_SUBSET (this);
    if (context->plan->drop_hints)
    {
      // AnchorFormat 2 and 3 just containing extra hinting information, so
      // if hints are being dropped convert to format 1.
      if (u.format != 1 && u.format != 2 && u.format != 3)
        return_trace (false);
      return_trace (bool (reinterpret_cast<Anchor *> (u.format1.copy (context->serializer))));
    }

    switch (u.format) {
    case 1: return_trace (bool (reinterpret_cast<Anchor *> (u.format1.copy (context->serializer))));
    case 2: return_trace (bool (reinterpret_cast<Anchor *> (u.format2.copy (context->serializer))));
    case 3: return_trace (bool (reinterpret_cast<Anchor *> (u.format3.copy (context->serializer,
                                                                              context->plan->layout_variation_idx_map))));
    default:return_trace (false);
    }
  }