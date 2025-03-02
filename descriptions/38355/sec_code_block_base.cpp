if (unlikely (get_type () == TSubTable::Extension && subtables && !c->get_edit_count ()))
    {
      /* The spec says all subtables of an Extension lookup should
       * have the same type, which shall not be the Extension type
       * itself (but we already checked for that).
       * This is specially important if one has a reverse type!
       *
       * We only do this if sanitizer edit_count is zero.  Otherwise,
       * some of the subtables might have become insane after they
       * were sanity-checked by the edits of subsequent subtables.
       * https://bugs.chromium.org/p/chromium/issues/detail?id=960331
       */
      unsigned int type = get_subtable<TSubTable> (0).u.extension.get_type ();
      for (unsigned int i = 1; i < subtables; i++)
	if (get_subtable<TSubTable> (i).u.extension.get_type () != type)
	  return_trace (false);
    }