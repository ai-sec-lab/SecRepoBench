inline bool sanitize (hb_sanitize_context_t *context,
			unsigned int *num_entries_out = nullptr) const
  {
    TRACE_SANITIZE (this);
    if (unlikely (!(context->check_struct (this) &&
		    classTable.sanitize (context, this)))) return_trace (false);

    const HBUINT16 *states = (this+stateArrayTable).arrayZ;
    const Entry<Extra> *entries = (this+entryTable).arrayZ;

    // <MASK>

    if (num_entries_out)
      *num_entries_out = num_entries;

    return_trace (true);
  }