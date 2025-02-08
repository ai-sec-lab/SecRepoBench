if (glyphids.is_empty ()) return_trace (0);
    unsigned count  = 0;

    hb_codepoint_t start_gid= glyphids.get_min ();
    hb_codepoint_t prev_gid = start_gid;

    unsigned offset = gid_offset_map.get (start_gid);
    unsigned prev_offset = offset;
    for (const hb_codepoint_t _ : glyphids.iter ())
    {
      if (_ == start_gid) continue;
      
      offset = gid_offset_map.get (_);
      if (_ == prev_gid + 1 &&  offset == prev_offset)
      {
        prev_gid = _;
        continue;
      }

      ClipRecord record;
      record.startGlyphID = start_gid;
      record.endGlyphID = prev_gid;
      record.clipBox = prev_offset;

      if (!c->copy (record, this)) return_trace (0);
      count++;

      start_gid = _;
      prev_gid = _;
      prev_offset = offset;
    }

    //last one
    {
      ClipRecord record;
      record.startGlyphID = start_gid;
      record.endGlyphID = prev_gid;
      record.clipBox = prev_offset;
      if (!c->copy (record, this)) return_trace (0);
      count++;
    }
    return_trace (count);