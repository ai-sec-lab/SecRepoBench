if (!Types::extended)
      return_trace (substitutionTables.sanitize (c, this, 0));

    unsigned int num_lookups = 0;

    const Entry<EntryData> *entries = machine.get_entries ();
    for (unsigned int i = 0; i < entrycount; i++)
    {
      const EntryData &data = entries[i].data;

      if (data.markIndex != 0xFFFF)
	num_lookups = MAX<unsigned int> (num_lookups, 1 + data.markIndex);
      if (data.currentIndex != 0xFFFF)
	num_lookups = MAX<unsigned int> (num_lookups, 1 + data.currentIndex);
    }

    return_trace (substitutionTables.sanitize (c, this, num_lookups));