for (const auto gid_klass_pair : + (++iterator))
    {
      hb_codepoint_t cur_gid = gid_klass_pair.first;
      unsigned cur_klass = gid_klass_pair.second;

      if (cur_gid != prev_gid + 1 ||
	  cur_klass != prev_klass)
      {
	if (unlikely (!record)) break;
	record->last = prev_gid;
	num_ranges++;

	range_rec.first = cur_gid;
	range_rec.last = cur_gid;
	range_rec.value = cur_klass;

	record = c->copy (range_rec);
      }

      prev_klass = cur_klass;
      prev_gid = cur_gid;
    }

    if (likely (record)) record->last = prev_gid;
    rangeRecord.len = num_ranges;
    rangeRecord.as_array ().qsort (RangeRecord<Types>::cmp_range); // To handle unsorted glyph order.

    return_trace (true);