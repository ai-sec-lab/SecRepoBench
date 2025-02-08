char *out = (char *) xmalloc (length + 1);
  out[length] = 0;
  return (char *) memcpy (out, in, length);