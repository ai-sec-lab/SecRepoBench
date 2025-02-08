char *out = (char *) xmalloc (len + 1);
  out[len] = 0;
  return (char *) memcpy (out, in, len);