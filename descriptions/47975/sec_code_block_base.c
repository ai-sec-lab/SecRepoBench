{
	  char *p = fgets (buf, sizeof (buf), f_in);
	  if (p && startswith (p, "O_APP") && ISSPACE (p[5]))
	    preprocess = 0;
	  if (!p || !strchr (p, '\n'))
	    ungetc ('#', f_in);
	  else
	    ungetc ('\n', f_in);
	}
      else if (c == 'A')
	{
	  char *p = fgets (buf, sizeof (buf), f_in);
	  if (p && startswith (p, "PP") && ISSPACE (p[2]))
	    preprocess = 1;
	  if (!p || !strchr (p, '\n'))
	    ungetc ('#', f_in);
	  else
	    ungetc ('\n', f_in);
	}
      else if (c == '\n')
	ungetc ('\n', f_in);
      else
	ungetc ('#', f_in);