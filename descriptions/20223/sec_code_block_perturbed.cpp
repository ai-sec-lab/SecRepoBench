hb_blob_ptr_t<sbix> table = hb_sanitize_context_t().reference_table<sbix> (context->plan->source);
    const unsigned int sbix_len = table.get_blob ()->length;
    table.destroy ();

    return_trace (serialize_strike_offsets (context, sbix_prime, sbix_len));