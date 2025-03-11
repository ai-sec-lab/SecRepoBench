bool serialize (hb_serialize_context_t *c,
		  Iterator iterator)
  {
    TRACE_SERIALIZE (this);
    if (unlikely (!c->extend_min (this))) return_trace (false);

    if (unlikely (!iterator))
    {
      classFormat = 2;
      rangeRecord.len = 0;
      return_trace (true);
    }

    unsigned num_ranges = 1;
    hb_codepoint_t prev_gid = (*iterator).first;
    unsigned prev_klass = (*iterator).second;

    RangeRecord<Types> range_rec;
    range_rec.first = prev_gid;
    range_rec.last = prev_gid;
    range_rec.value = prev_klass;

    auto *record = c->copy (range_rec);
    if (unlikely (!record)) return_trace (false);

    // <MASK>
  }