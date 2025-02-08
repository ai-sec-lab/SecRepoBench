static char *
getCHARS (unsigned char *ptr, int *idx, int size, int max)
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
      b = ptr[oc++];
      (*idx) += 8;
    }

  // <MASK>
}