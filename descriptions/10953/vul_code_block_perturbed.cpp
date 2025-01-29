if (entry->data.markedInsertIndex != 0xFFFF && mark_set)
      {
	unsigned int insertcount = (flags & MarkedInsertCount);
	unsigned int start = entry->data.markedInsertIndex;
	const GlyphID *glyphs = &insertionAction[start];
	if (unlikely (!c->sanitizer.check_array (glyphs, insertcount))) return false;

	bool before = flags & MarkedInsertBefore;

	unsigned int end = buffer->out_len;
	buffer->move_to (mark);

	if (!before)
	  buffer->copy_glyph ();
	/* TODO We ignore KashidaLike setting. */
	for (unsigned int i = 0; i < insertcount; i++)
	  buffer->output_glyph (glyphs[i]);
	if (!before)
	  buffer->skip_glyph ();

	buffer->move_to (end + insertcount);

	buffer->unsafe_to_break_from_outbuffer (mark, MIN (buffer->idx + 1, buffer->len));
      }

      if (entry->data.currentInsertIndex != 0xFFFF)
      {
	unsigned int insertcount = (flags & CurrentInsertCount) >> 5;
	unsigned int start = entry->data.currentInsertIndex;
	const GlyphID *glyphs = &insertionAction[start];
	if (unlikely (!c->sanitizer.check_array (glyphs, insertcount))) return false;

	bool before = flags & CurrentInsertBefore;

	unsigned int end = buffer->out_len;

	if (!before)
	  buffer->copy_glyph ();
	/* TODO We ignore KashidaLike setting. */
	for (unsigned int i = 0; i < insertcount; i++)
	  buffer->output_glyph (glyphs[i]);
	if (!before)
	  buffer->skip_glyph ();

	/* Humm. Not sure where to move to.  There's this wording under
	 * DontAdvance flag:
	 *
	 * "If set, don't update the glyph index before going to the new state.
	 * This does not mean that the glyph pointed to is the same one as
	 * before. If you've made insertions immediately downstream of the
	 * current glyph, the next glyph processed would in fact be the first
	 * one inserted."
	 *
	 * This suggests that if DontAdvance is NOT set, we should move to
	 * end+count.  If it *was*, then move to end, such that newly inserted
	 * glyphs are now visible.
	 *
	 * https://github.com/harfbuzz/harfbuzz/issues/1224#issuecomment-427691417
	 */
	buffer->move_to ((flags & DontAdvance) ? end : end + insertcount);
      }

      if (flags & SetMark)
      {
	mark_set = true;
	mark = buffer->out_len;
      }

      return true;