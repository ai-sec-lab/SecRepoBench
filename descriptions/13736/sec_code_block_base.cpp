)
      if (fdmap.includes (i))
      {
      	if (unlikely (fid >= fdCount)) return_trace (false);
	CFFIndexOf<COUNT, FontDict>::set_offset_at (fid++, offset);
	offset += FontDict::calculate_serialized_size (fontDicts[i], opszr);
      }