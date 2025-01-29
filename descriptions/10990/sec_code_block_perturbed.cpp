if (buffer->idx >= buffer->len)
	    return false; // TODO Work on previous instead?
	  unsigned int component_idx = buffer->cur().codepoint + offset;

	  const HBUINT16 &componentData = component[component_idx];
	  if (unlikely (!componentData.sanitize (&c->sanitizer))) return false;
	  ligature_idx += componentData;

	  if (action & (LigActionStore | LigActionLast))
	  {
	    const GlyphID &ligatureData = ligature[ligature_idx];
	    if (unlikely (!ligatureData.sanitize (&c->sanitizer))) return false;
	    hb_codepoint_t lig = ligatureData;

	    match_positions[match_length++] = buffer->out_len;
	    buffer->replace_glyph (lig);

	    //ligature_idx = 0; // XXX Yes or no?
	  }
	  else
	  {
	    buffer->skip_glyph ();
	    outlenend--;
	  }

	  action_idx++;