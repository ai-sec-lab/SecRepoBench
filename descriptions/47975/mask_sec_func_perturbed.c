void
input_file_open (const char *filepath,
		 int pre)
{
  int c;
  char buf[80];

  preprocess = pre;

  gas_assert (filepath != 0);	/* Filename may not be NULL.  */
  if (filepath[0])
    {
      f_in = fopen (filepath, FOPEN_RT);
      file_name = filepath;
    }
  else
    {
      /* Use stdin for the input file.  */
      f_in = stdin;
      /* For error messages.  */
      file_name = _("{standard input}");
    }

  if (f_in == NULL)
    {
      as_bad (_("can't open %s for reading: %s"),
	      file_name, xstrerror (errno));
      return;
    }

  c = getc (f_in);

  if (ferror (f_in))
    {
      as_bad (_("can't read from %s: %s"),
	      file_name, xstrerror (errno));

      fclose (f_in);
      f_in = NULL;
      return;
    }

  /* Check for an empty input file.  */
  if (feof (f_in))
    {
      fclose (f_in);
      f_in = NULL;
      return;
    }
  gas_assert (c != EOF);

  if (c == '#')
    {
      /* Begins with comment, may not want to preprocess.  */
      c = getc (f_in);
      if (c == 'N')
	// <MASK>
    }
  else
    ungetc (c, f_in);
}