bool sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    if (!(c->check_struct (this) && subTable.sanitize (c))) return_trace (false);

    unsigned subtables = get_subtable_count ();
    if (unlikely (!c->visit_subtables (subtables))) return_trace (false);

    if (lookupFlag & LookupFlag::UseMarkFilteringSet)
    {
      const HBUINT16 &markFilteringSet = StructAfter<HBUINT16> (subTable);
      if (!markFilteringSet.sanitize (c)) return_trace (false);
    }

    if (unlikely (!get_subtables<TSubTable> ().sanitize (c, this, get_type ())))
      return_trace (false);

    // Check if the type is an Extension and edit_count is zero.
    // If so, ensure that all subtables of the Extension lookup have the same type.
    // If any subtable has a different type, return false, indicating a failure.
    // <MASK>
    return_trace (true);
  }