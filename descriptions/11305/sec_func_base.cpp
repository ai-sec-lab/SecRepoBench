inline bool transition (StateTableDriver<Types, EntryData> *driver,
			    const Entry<EntryData> *entry)
    {
      hb_buffer_t *buffer = driver->buffer;
      unsigned int flags = entry->flags;

      if (flags & Format1EntryT::Reset)
	depth = 0;

      if (flags & Format1EntryT::Push)
      {
	if (likely (depth < ARRAY_LENGTH (stack)))
	  stack[depth++] = buffer->idx;
	else
	  depth = 0; /* Probably not what CoreText does, but better? */
      }

      if (Format1EntryT::performAction (entry))
      {
	unsigned int tuple_count = MAX (1u, table->header.tuple_count ());

	unsigned int kern_idx = Format1EntryT::kernActionIndex (entry);
	kern_idx = Types::offsetToIndex (kern_idx, &table->machine, kernAction.arrayZ);
	const FWORD *actions = &kernAction[kern_idx];
	if (!c->sanitizer.check_array (actions, depth * tuple_count))
	{
	  depth = 0;
	  return false;
	}

	hb_mask_t kern_mask = c->plan->kern_mask;

	/* From Apple 'kern' spec:
	 * "Each pops one glyph from the kerning stack and applies the kerning value to it.
	 * The end of the list is marked by an odd value... */
	bool last = false;
	while (!last && depth--)
	{
	  unsigned int idx = stack[depth];
	  int v = *actions;
	  actions += tuple_count;
	  if (idx >= buffer->len) continue;

	  /* "The end of the list is marked by an odd value..." */
	  last = v & 1;
	  v &= ~1;

	  hb_glyph_position_t &o = buffer->pos[idx];

	  /* Testing shows that CoreText only applies kern (cross-stream or not)
	   * if none has been applied by previous subtables.  That is, it does
	   * NOT seem to accumulate as otherwise implied by specs. */

	  /* The following flag is undocumented in the spec, but described
	   * in the 'kern' table example. */
	  if (v == -0x8000)
	  {
	    o.attach_type() = ATTACH_TYPE_NONE;
	    o.attach_chain() = 0;
	    o.x_offset = o.y_offset = 0;
	  }
	  else if (HB_DIRECTION_IS_HORIZONTAL (buffer->props.direction))
	  {
	    if (crossStream)
	    {
	      if (buffer->pos[idx].attach_type() && !buffer->pos[idx].y_offset)
	      {
		o.y_offset = c->font->em_scale_y (v);
		buffer->scratch_flags |= HB_BUFFER_SCRATCH_FLAG_HAS_GPOS_ATTACHMENT;
	      }
	    }
	    else if (buffer->info[idx].mask & kern_mask)
	    {
	      if (!buffer->pos[idx].x_offset)
	      {
		buffer->pos[idx].x_advance += c->font->em_scale_x (v);
		buffer->pos[idx].x_offset += c->font->em_scale_x (v);
	      }
	    }
	  }
	  else
	  {
	    if (crossStream)
	    {
	      /* CoreText doesn't do crossStream kerning in vertical.  We do. */
	      if (buffer->pos[idx].attach_type() && !buffer->pos[idx].x_offset)
	      {
		o.x_offset = c->font->em_scale_x (v);
		buffer->scratch_flags |= HB_BUFFER_SCRATCH_FLAG_HAS_GPOS_ATTACHMENT;
	      }
	    }
	    else if (buffer->info[idx].mask & kern_mask)
	    {
	      if (!buffer->pos[idx].y_offset)
	      {
		buffer->pos[idx].y_advance += c->font->em_scale_y (v);
		buffer->pos[idx].y_offset += c->font->em_scale_y (v);
	      }
	    }
	  }
	}
      }

      return true;
    }