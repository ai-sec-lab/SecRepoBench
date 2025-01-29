TRACE_SERIALIZE (this);
    if (gids.is_empty ()) return_trace (0);
    unsigned count  = 0;

    hb_codepoint_t start_gid= gids.get_min ();