bool sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    return_trace (c->check_struct (this) &&
		  likely (format == 0 || format == 1) &&
		  c->check_array (nameRecordZ.arrayZ, count) &&
		  c->check_range (this, stringOffset) &&
		  sanitize_records (c));
  }