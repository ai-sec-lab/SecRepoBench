if (!unlikely (c->allocate_size<HBUINT8> (offsetSize * (fdCount + 1))))
      return_trace (false);

    /* serialize font dict offsets */
    unsigned int  offset = 1;
    unsigned int  fid = 0;
    for (unsigned i = 0; i < fontDicts.length; i++)
      if (fdmap.includes (i))
      {
      	if (unlikely (fid >= fdCount)) return_trace (false);
	CFFIndexOf<COUNT, FontDict>::set_offset_at (fid++, offset);
	offset += FontDict::calculate_serialized_size (fontDicts[i], opszr);
      }
    CFFIndexOf<COUNT, FontDict>::set_offset_at (fid, offset);

    /* serialize font dicts */