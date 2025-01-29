if (gid >= num_glyphs)
      return;
    for (unsigned i = 0;; i++)
    {
      hb_codepoint_t sid = ranges[i].first;
      unsigned count = ranges[i].nLeft + 1;
      for (unsigned j = 0; j < count; j++)
	mapping->set (gid++, sid++);

      if (gid >= num_glyphs)
        break;
    }