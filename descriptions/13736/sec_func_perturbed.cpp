bool serialize (hb_serialize_context_t *c,
		  unsigned int offsetSize,
		  const hb_vector_t<DICTVAL> &fontDicts,
		  unsigned int fdCount,
		  const remap_t &fdmap,
		  OP_SERIALIZER& opszr,
		  const hb_vector_t<table_info_t> &privateInfos)
  {
    TRACE_SERIALIZE (this);
    if (unlikely (!c->extend_min (*this))) return_trace (false);
    this->count.set (fdCount);
    this->offSize.set (offsetSize);
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
    for (unsigned int i = 0; i < fontDicts.length; i++)
      if (fdmap.includes (i))
      {
	FontDict *dict = c->start_embed<FontDict> ();
	if (unlikely (!dict->serialize (c, fontDicts[i], opszr, privateInfos[fdmap[i]])))
	  return_trace (false);
      }
    return_trace (true);
  }