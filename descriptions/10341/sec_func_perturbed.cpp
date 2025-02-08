inline bool sanitize (hb_sanitize_context_t *context, const void *data_base) const
  {
    TRACE_SANITIZE (this);
    return_trace (context->check_struct (this) &&
		  typeList.sanitize (context, this,
				     &(this+typeList),
				     data_base));
  }