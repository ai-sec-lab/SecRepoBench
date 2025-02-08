unsigned int num_states = 1;
    unsigned int num_entries = 0;

    unsigned int state = 0;
    unsigned int entry = 0;
    while (state < num_states)
    {
      if (unlikely (!c->check_array (states,
				     num_states,
				     states[0].static_size * nClasses)))
	return_trace (false);
      { /* Sweep new states. */
	const HBUINT16 *stop = &states[num_states * nClasses];
	for (const HBUINT16 *p = &states[state * nClasses]; p < stop; p++)
	  num_entries = MAX<unsigned int> (num_entries, *p + 1);
	state = num_states;
      }

      if (unlikely (!c->check_array (entries, num_entries)))
	return_trace (false);
      { /* Sweep new entries. */
	const Entry<Extra> *stop = &entries[num_entries];
	for (const Entry<Extra> *p = &entries[entry]; p < stop; p++)
	  num_states = MAX<unsigned int> (num_states, p->newState + 1);
	entry = num_entries;
      }
    }