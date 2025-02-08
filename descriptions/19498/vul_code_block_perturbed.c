
      ++lineptr;
      ++col;

      auto process_tab = [&] ()
	{
	  int max_tab_len = tui_tab_width;

	  --col;
	  for (int j = col % max_tab_len;
	       j < max_tab_len && col < first_col + line_width;
	       col++, j++)
	    if (col >= first_col)
	      result.push_back (' ');
	};

      /* We have to process all the text in order to pick up all the
	 escapes.  */
      if (col <= first_col || col > first_col + line_width)
	{
	  if (c == '\t')
	    process_tab ();
	  continue;
	}

      if (c == '\n' || c == '\r' || c == '\0')
	{
	  /* Nothing.  */
	}
      else if (c < 040 && c != '\t')
	{
	  result.push_back ('^');
	  result.push_back (c + 0100);
	}
      else if (c == 0177)
	{
	  result.push_back ('^');
	  result.push_back ('?');
	}
      else if (c == '\t')
	process_tab ();
      else
	result.push_back (c);