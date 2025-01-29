int tail_len = init_tail - c->tail;
    c->check_assign (this->length, c->length () - table_initpos + tail_len);
    c->check_assign (this->record.len,
		     (c->length () - table_initpos - CmapSubtableFormat14::min_size) /
		     VariationSelectorRecord::static_size);

    /* Correct the incorrect write order by reversing the order of the variation
       records array. */
    _reverse_variation_records ();

    /* Now that records are in the right order, we can set up the offsets. */
    _add_links_to_variation_records (c, obj_indices);