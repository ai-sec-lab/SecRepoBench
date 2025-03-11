bool sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    if (!(c->check_struct (this) && subTable.sanitize (c))) return_trace (false);

    unsigned subtablecount = get_subtable_count ();
    if (unlikely (!c->visit_subtables (subtablecount))) return_trace (false);

    if (lookupFlag & LookupFlag::UseMarkFilteringSet)
    {
      const HBUINT16 &markFilteringSet = StructAfter<HBUINT16> (subTable);
      if (!markFilteringSet.sanitize (c)) return_trace (false);
    }

    if (unlikely (!get_subtables<TSubTable> ().sanitize (c, this, get_type ())))
      return_trace (false);

    if (unlikely (get_type () == TSubTable::Extension && subtablecount && !c->get_edit_count ()))
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
      for (unsigned int i = 1; i < subtablecount; i++)
	if (get_subtable<TSubTable> (i).u.extension.get_type () != type)
	  return_trace (false);
    }
    return_trace (true);
  }