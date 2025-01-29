TRACE_SERIALIZE (this);
    if (glyphids.is_empty ()) return_trace (0);
    unsigned count  = 0;

    hb_codepoint_t start_gid= glyphids.get_min ();