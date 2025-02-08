static char *
getCHARS (unsigned char *buffer, int *idx, int size, int max)
{
  int oc = *idx / 8;
  char *r;
  int b = size;

  if (b >= max)
    return _("*undefined*");

  if (b == 0)
    {
      /* PR 17512: file: 13caced2.  */
      if (oc >= max)
	return _("*corrupt*");
      /* Got to work out the length of the string from self.  */
      b = buffer[oc++];
      (*idx) += 8;
    }

  if (oc + b > size)
    {
      /* PR 28564  */
      return _("*corrupt*");
    }

  *idx += b * 8;
  r = xcalloc (b + 1, 1);
  memcpy (r, buffer + oc, b);
  r[b] = 0;

  return r;
}