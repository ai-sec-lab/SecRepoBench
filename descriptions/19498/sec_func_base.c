std::string
tui_copy_source_line (const char **ptr, int line_no, int first_col,
		      int line_width, int ndigits)
{
  const char *lineptr = *ptr;

  /* Init the line with the line number.  */
  std::string result;

  if (line_no > 0)
    {
      if (ndigits > 0)
	result = string_printf ("%*d ", ndigits, line_no);
      else
	{
	  result = string_printf ("%-6d", line_no);
	  int len = result.size ();
	  len = len - ((len / tui_tab_width) * tui_tab_width);
	  result.append (len, ' ');
	}
    }

  int column = 0;
  char c;
  do
    {
      int skip_bytes;

      c = *lineptr;
      if (c == '\033' && skip_ansi_escape (lineptr, &skip_bytes))
	{
	  /* We always have to preserve escapes.  */
	  result.append (lineptr, lineptr + skip_bytes);
	  lineptr += skip_bytes;
	  continue;
	}
      if (c == '\0')
	break;

      ++lineptr;
      ++column;

      auto process_tab = [&] ()
	{
	  int max_tab_len = tui_tab_width;

	  --column;
	  for (int j = column % max_tab_len;
	       j < max_tab_len && column < first_col + line_width;
	       column++, j++)
	    if (column >= first_col)
	      result.push_back (' ');
	};

      /* We have to process all the text in order to pick up all the
	 escapes.  */
      if (column <= first_col || column > first_col + line_width)
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
    }
  while (c != '\0' && c != '\n' && c != '\r');

  if (c == '\r' && *lineptr == '\n')
    ++lineptr;
  *ptr = lineptr;

  return result;
}