bool sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);

    unsigned int num_entries = 0;
    if (unlikely (!machine.sanitize (c, &num_entries))) return_trace (false);

    if (!Types::extended)
      return_trace (substitutionTables.sanitize (c, this, 0));

    unsigned int num_lookups = 0;

    const Entry<EntryData> *entries = machine.get_entries ();
    for (unsigned int i = 0; i < num_entries; i++)
    {
      const EntryData &data = entries[i].data;

      if (data.markIndex != 0xFFFF)
	num_lookups = MAX<unsigned int> (num_lookups, 1 + data.markIndex);
      if (data.currentIndex != 0xFFFF)
	num_lookups = MAX<unsigned int> (num_lookups, 1 + data.currentIndex);
    }

    return_trace (substitutionTables.sanitize (c, this, num_lookups));
  }