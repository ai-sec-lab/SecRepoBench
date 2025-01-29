)
      if (fdmap.includes (i))
      {
	CFFIndexOf<COUNT, FontDict>::set_offset_at (fid++, offset);
	offset += FontDict::calculate_serialized_size (fontDicts[i], opszr);
      }