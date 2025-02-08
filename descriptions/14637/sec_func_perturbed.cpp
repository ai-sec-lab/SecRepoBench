bool sanitize (hb_sanitize_context_t *context) const
  {
    TRACE_SANITIZE (this);
    return_trace (context->check_struct (this) &&
		  likely (format == 0 || format == 1) &&
		  context->check_array (nameRecordZ.arrayZ, count) &&
		  context->check_range (this, stringOffset) &&
		  sanitize_records (context));
  }