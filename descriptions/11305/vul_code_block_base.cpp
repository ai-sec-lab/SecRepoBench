unsigned int kern_idx = Format1EntryT::kernActionIndex (entry);
	kern_idx = Types::offsetToIndex (kern_idx, &table->machine, kernAction.arrayZ);
	const FWORD *actions = &kernAction[kern_idx];
	if (!c->sanitizer.check_array (actions, depth))
	{
	  depth = 0;
	  return false;
	}

	hb_mask_t kern_mask = c->plan->kern_mask;

	/* From Apple 'kern' spec:
	 * "Each pops one glyph from the kerning stack and applies the kerning value to it.
	 * The end of the list is marked by an odd value... */
	unsigned int tuple_count = table->header.tuple_count ();
	tuple_count = tuple_count ? tuple_count : 1;
	bool last = false;