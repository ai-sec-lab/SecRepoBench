)
  {
    unsigned seqIndex = lookupRecord[i].sequenceIndex;
    if (seqIndex > inputCount) continue;

    hb_set_t *pos_glyphs = hb_set_create ();

    if (hb_set_is_empty (covered_seq_indicies) || !hb_set_has (covered_seq_indicies, seqIndex))
    {
      if (seqIndex == 0)
      {
        switch (context_format) {
        case ContextFormat::SimpleContext:
          pos_glyphs->add (value);
          break;
        case ContextFormat::ClassBasedContext:
          intersected_glyphs_func (c->cur_intersected_glyphs, contextdata, value, pos_glyphs);
          break;
        case ContextFormat::CoverageBasedContext:
          hb_set_set (pos_glyphs, c->cur_intersected_glyphs);
          break;
        }
      }
      else
      {
        const void *input_data = input;
        unsigned input_value = seqIndex - 1;
        if (context_format != ContextFormat::SimpleContext)
        {
          input_data = contextdata;
          input_value = input[seqIndex - 1];
        }

        intersected_glyphs_func (c->glyphs, input_data, input_value, pos_glyphs);
      }
    }

    hb_set_add (covered_seq_indicies, seqIndex);
    c->push_cur_active_glyphs (pos_glyphs);

    unsigned endIndex = inputCount;
    if (context_format == ContextFormat::CoverageBasedContext)
      endIndex += 1;

    c->recurse (lookupRecord[i].lookupListIndex, covered_seq_indicies, seqIndex, endIndex);

    c->pop_cur_done_glyphs ();
    hb_set_destroy (pos_glyphs);
  }