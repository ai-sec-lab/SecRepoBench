inline bool sanitize (hb_sanitize_context_t *c,
			unsigned int *num_entries_out = nullptr) const
  {
    TRACE_SANITIZE (this);
    if (unlikely (!(c->check_struct (this) &&
		    classTable.sanitize (c, this)))) return_trace (false);

    const HBUINT16 *states = (this+stateArrayTable).arrayZ;
    const Entry<Extra> *entries = (this+entryTable).arrayZ;

    // <MASK>

    if (num_entries_out)
      *num_entries_out = num_entries;

    return_trace (true);
  }