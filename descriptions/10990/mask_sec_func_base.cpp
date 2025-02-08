inline bool transition (StateTableDriver<EntryData> *driver,
			    const Entry<EntryData> *entry)
    {
      hb_buffer_t *buffer = driver->buffer;
      unsigned int flags = entry->flags;

      if (flags & SetComponent)
      {
        if (unlikely (match_length >= ARRAY_LENGTH (match_positions)))
	  return false;

	/* Never mark same index twice, in case DontAdvance was used... */
	if (match_length && match_positions[match_length - 1] == buffer->out_len)
	  match_length--;

	match_positions[match_length++] = buffer->out_len;
      }

      if (flags & PerformAction)
      {
	unsigned int end = buffer->out_len;
	unsigned int action_idx = entry->data.ligActionIndex;
	unsigned int action;
	unsigned int ligature_idx = 0;

	if (unlikely (!match_length))
	  return false;

	buffer->merge_out_clusters (match_positions[0], buffer->out_len);

        do
	{
	  if (unlikely (!match_length))
	    return false;

	  buffer->move_to (match_positions[--match_length]);

	  const HBUINT32 &actionData = ligAction[action_idx];
	  if (unlikely (!actionData.sanitize (&c->sanitizer))) return false;
	  action = actionData;

	  uint32_t uoffset = action & LigActionOffset;
	  if (uoffset & 0x20000000)
	    uoffset += 0xC0000000;
	  int32_t offset = (int32_t) uoffset;
	  // <MASK>
	}
	while (!(action & LigActionLast));
	buffer->move_to (end);
      }

      return true;
    }