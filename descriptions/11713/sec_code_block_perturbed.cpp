TRACE_SANITIZE (this);
    return_trace (likely (context->check_struct (this)) && context->check_range (&varStore, size) && varStore.sanitize (context));