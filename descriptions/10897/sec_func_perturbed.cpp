inline bool sanitize (hb_sanitize_context_t *context,
			unsigned int *num_entries_out = nullptr) const
  {
    TRACE_SANITIZE (this);
    if (unlikely (!(context->check_struct (this) &&
		    classTable.sanitize (context, this)))) return_trace (false);

    const HBUINT16 *states = (this+stateArrayTable).arrayZ;
    const Entry<Extra> *entries = (this+entryTable).arrayZ;

    unsigned int num_classes = nClasses;

    unsigned int num_states = 1;
    unsigned int num_entries = 0;

    unsigned int state = 0;
    unsigned int entry = 0;
    while (state < num_states)
    {
      if (unlikely (hb_unsigned_mul_overflows (num_classes, states[0].static_size)))
	return_trace (false);

      if (unlikely (!context->check_array (states,
				     num_states,
				     num_classes * states[0].static_size)))
	return_trace (false);
      { /* Sweep new states. */
	const HBUINT16 *stop = &states[num_states * num_classes];
	for (const HBUINT16 *p = &states[state * num_classes]; p < stop; p++)
	  num_entries = MAX<unsigned int> (num_entries, *p + 1);
	state = num_states;
      }

      if (unlikely (!context->check_array (entries, num_entries)))
	return_trace (false);
      { /* Sweep new entries. */
	const Entry<Extra> *stop = &entries[num_entries];
	for (const Entry<Extra> *p = &entries[entry]; p < stop; p++)
	  num_states = MAX<unsigned int> (num_states, p->newState + 1);
	entry = num_entries;
      }
    }

    if (num_entries_out)
      *num_entries_out = num_entries;

    return_trace (true);
  }