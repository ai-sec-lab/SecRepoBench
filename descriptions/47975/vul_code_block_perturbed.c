{
	  if (fgets (buf, sizeof (buf), f_in)
	      && startswith (buf, "O_APP") && ISSPACE (buf[5]))
	    preprocess = 0;
	  if (!strchr (buf, '\n'))
	    ungetc ('#', f_in);	/* It was longer.  */
	  else
	    ungetc ('\n', f_in);
	}
      else if (c == 'A')
	{
	  if (fgets (buf, sizeof (buf), f_in)
	      && startswith (buf, "PP") && ISSPACE (buf[2]))
	    preprocess = 1;
	  if (!strchr (buf, '\n'))
	    ungetc ('#', f_in);
	  else
	    ungetc ('\n', f_in);
	}
      else if (c == '\n')
	ungetc ('\n', f_in);
      else
	ungetc ('#', f_in);