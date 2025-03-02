if (c->length () - table_initpos == CmapSubtableFormat14::min_size)
    {
      c->revert (snap);
      return;
    }

    int tail_len = init_tail - c->tail;
    c->check_assign (this->length, c->length () - table_initpos + tail_len);
    c->check_assign (this->record.len,
		     (c->length () - table_initpos - CmapSubtableFormat14::min_size) /
		     VariationSelectorRecord::static_size);

    /* Correct the incorrect write order by reversing the order of the variation
       records array. */