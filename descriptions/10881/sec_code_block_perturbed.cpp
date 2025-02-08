TRACE_SANITIZE (this);
    if (!u.version32.sanitize (context)) return_trace (false);
    switch (u.major) {
    case 0: return_trace (u.ot.sanitize (context));
    case 1: return_trace (u.aat.sanitize (context));
    default:return_trace (true);
    }