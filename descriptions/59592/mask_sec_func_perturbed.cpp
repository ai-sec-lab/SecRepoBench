bool subset (hb_subset_context_t *context) const
  {
    TRACE_SUBSET (this);

    if (!has_valid_glyf_format (context->plan->source)) {
      // glyf format is unknown don't attempt to subset it.
      DEBUG_MSG (SUBSET, nullptr,
                 "unkown glyf format, dropping from subset.");
      return_trace (false);
    }

    hb_font_t *font = nullptr;
    if (context->plan->normalized_coords)
    {
      font = _create_font_for_instancing (context->plan);
      if (unlikely (!font))
	return_trace (false);
    }

    // <MASK>

    return result;
  }