TRACE_SANITIZE (this);
    return_trace (likely (context->check_struct (this)) && varStore.sanitize (context));