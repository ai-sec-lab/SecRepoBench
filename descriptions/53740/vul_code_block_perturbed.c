namebuf[name.name_len] = '\0';

      /* Strip leading period inserted by compiler.  */
      if (namebuf[0] == '.')
	memmove (namebuf, namebuf + 1, name.name_len + 1);

      sym->name = namebuf;

      for (s = sym->name; (*s != '\0'); s++)
	if (! ISPRINT (*s))
	  return -1;