static inline bool apply_lookup (hb_apply_context_t *c,
				 unsigned int count, /* Including the first glyph */
				 unsigned int match_positions[HB_MAX_CONTEXT_LENGTH], /* Including the first glyph */
				 unsigned int lookupCount,
				 const LookupRecord lookupRecord[], /* Array of LookupRecords--in design order */
				 unsigned int match_length)
{
  TRACE_APPLY (nullptr);

  hb_buffer_t *buffer = c->buffer;
  int end;

  /* All positions are distance from beginning of *output* buffer.
   * Adjust. */
  {
    unsigned int backtracklength = buffer->backtrack_len ();
    end = backtracklength + match_length;

    int delta = backtracklength - buffer->idx;
    /* Convert positions to new indexing. */
    for (unsigned int j = 0; j < count; j++)
      match_positions[j] += delta;
  }

  for (unsigned int i = 0; i < lookupCount && !buffer->in_error; i++)
  {
    unsigned int idx = lookupRecord[i].sequenceIndex;
    if (idx >= count)
      continue;

    /* Don't recurse to ourself at same position.
     * Note that this test is too naive, it doesn't catch longer loops. */
    if (idx == 0 && lookupRecord[i].lookupListIndex == c->lookup_index)
      continue;

    // <MASK>
  }

  buffer->move_to (end);

  return_trace (true);
}