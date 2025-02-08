inline bool sanitize (hb_sanitize_context_t *context) const
  {
    TRACE_SANITIZE (this);
    return_trace (context->check_struct (this) &&
		  context->check_range (kernValueZ,
				  kernValueCount * sizeof (FWORD) +
				  glyphCount * 2 +
				  leftClassCount * rightClassCount));
  }