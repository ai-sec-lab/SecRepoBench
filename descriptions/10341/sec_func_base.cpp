inline bool sanitize (hb_sanitize_context_t *c, const void *data_base) const
  {
    TRACE_SANITIZE (this);
    return_trace (c->check_struct (this) &&
		  typeList.sanitize (c, this,
				     &(this+typeList),
				     data_base));
  }